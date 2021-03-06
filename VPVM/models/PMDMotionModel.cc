/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2010-2012  hkrn                                    */
/*                                                                   */
/* All rights reserved.                                              */
/*                                                                   */
/* Redistribution and use in source and binary forms, with or        */
/* without modification, are permitted provided that the following   */
/* conditions are met:                                               */
/*                                                                   */
/* - Redistributions of source code must retain the above copyright  */
/*   notice, this list of conditions and the following disclaimer.   */
/* - Redistributions in binary form must reproduce the above         */
/*   copyright notice, this list of conditions and the following     */
/*   disclaimer in the documentation and/or other materials provided */
/*   with the distribution.                                          */
/* - Neither the name of the MMDAI project team nor the names of     */
/*   its contributors may be used to endorse or promote products     */
/*   derived from this software without specific prior written       */
/*   permission.                                                     */
/*                                                                   */
/* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND            */
/* CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,       */
/* INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF          */
/* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE          */
/* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS */
/* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,          */
/* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED   */
/* TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,     */
/* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON */
/* ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,   */
/* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY    */
/* OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE           */
/* POSSIBILITY OF SUCH DAMAGE.                                       */
/* ----------------------------------------------------------------- */

#include "common/StringHelper.h"
#include "common/util.h"
#include "models/PMDMotionModel.h"

#include <QtGui/QtGui>
#include <vpvl2/vpvl2.h>

namespace vpvm
{

using namespace vpvl2;

PMDMotionModel::State::State(const Scene *scene, IModel *model)
    : m_sceneRef(scene),
      m_modelRef(model)
{
}

PMDMotionModel::State::~State()
{
    m_sceneRef = 0;
    m_modelRef = 0;
}

void PMDMotionModel::State::restore() const
{
    m_modelRef->resetVertices();
    foreach (const Bone &bone, m_bones) {
        IBone *b = bone.first;
        const Transform &tr = bone.second;
        b->setLocalPosition(tr.first);
        b->setLocalRotation(tr.second);
    }
    foreach (const Morph &morph, m_morphs) {
        IMorph *m = morph.first;
        m->setWeight(morph.second);
    }
    m_sceneRef->updateModel(m_modelRef);
}

void PMDMotionModel::State::save()
{
    Array<IBone *> bones;
    m_modelRef->getBoneRefs(bones);
    m_bones.clear();
    const int nbones = bones.count();
    for (int i = 0; i < nbones; i++) {
        IBone *bone = bones[i];
        Transform tr(bone->localPosition(), bone->localRotation());
        m_bones.append(Bone(bone, tr));
    }
    Array<IMorph *> morphs;
    m_modelRef->getMorphRefs(morphs);
    m_morphs.clear();
    const int nmorphs = morphs.count();
    for (int i = 0; i < nmorphs; i++) {
        IMorph *morph = morphs[i];
        const Scalar &weight = morph->weight();
        if (!morph->hasParent())
            m_morphs.append(Morph(morph, weight));
    }
}

bool PMDMotionModel::State::compact()
{
    QMutableListIterator<Bone> bones(m_bones);
    while (bones.hasNext()) {
        bones.next();
        const Bone &value = bones.value();
        const IBone *bone = value.first;
        const Transform &transform = value.second;
        if (bone->localPosition() == transform.first && bone->localRotation() == transform.second)
            bones.remove();
    }
    QMutableListIterator<Morph> morphs(m_morphs);
    while (morphs.hasNext()) {
        morphs.next();
        const Morph &value = morphs.value();
        const IMorph *morph = value.first;
        const Scalar &weight = value.second;
        if (btFuzzyZero(morph->weight() - weight))
            morphs.remove();
    }
    return m_bones.size() > 0 || m_morphs.size() > 0;
}

void PMDMotionModel::State::discard()
{
    m_bones.clear();
    m_morphs.clear();
}

void PMDMotionModel::State::copyFrom(const State &value)
{
    m_bones = value.m_bones;
    m_morphs = value.m_morphs;
    m_modelRef = value.m_modelRef;
}

void PMDMotionModel::State::resetBones()
{
    Array<IBone *> bones;
    m_modelRef->getBoneRefs(bones);
    const int nbones = bones.count();
    for (int i = 0; i < nbones; i++) {
        IBone *bone = bones[i];
        bone->setLocalPosition(kZeroV3);
        bone->setLocalRotation(Quaternion::getIdentity());
    }
    m_sceneRef->updateModel(m_modelRef);
}

void PMDMotionModel::State::resetMorphs()
{
    Array<IMorph *> morphs;
    m_modelRef->resetVertices();
    m_modelRef->getMorphRefs(morphs);
    const int nmorphs = morphs.count();
    for (int i = 0; i < nmorphs; i++) {
        IMorph *morph = morphs[i];
        morph->setWeight(0);
    }
    m_sceneRef->updateModel(m_modelRef);
}

PMDMotionModel::PMDMotionModel(QUndoGroup *undoRef, QObject *parent) :
    MotionBaseModel(undoRef, parent),
    m_sceneRef(0),
    m_modelRef(0),
    m_lightDirection(kZeroV3)
{
    /* 空のモデルのデータを予め入れておく */
    m_roots.insert(0, RootPtr(0));
    m_keys.insert(0, Keys());
    m_values.insert(0, Values());
}

PMDMotionModel::~PMDMotionModel()
{
}

QVariant PMDMotionModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role == Qt::DisplayRole && index.column() == 0) {
        /* アイテムの名前(カテゴリ名とかボーン名とか頂点モーフ名とか)を返す */
        ITreeItem *item = static_cast<ITreeItem *>(index.internalPointer());
        return item->name();
    }
    else if (role == kBinaryDataRole && m_modelRef) {
        /* BaseKeyFrame#write によって書き出されたキーフレームのバイナリのデータを返す */
        QVariant value = m_values[m_modelRef].value(index);
        return value;
    }
    else {
        return QVariant();
    }
}

bool PMDMotionModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (m_modelRef && index.isValid() && role == Qt::EditRole) {
        m_values[m_modelRef].insert(index, value);
        setModified(true);
        emit dataChanged(index, index);
        return true;
    }
    return false;
}

const QModelIndex PMDMotionModel::frameIndexToModelIndex(ITreeItem *item, int frameIndex) const
{
    int rowIndex = item->rowIndex();
    /* カテゴリ名を含むアイテム */
    const QModelIndex &parentIndex = index(item->parent()->rowIndex(), 0);
    const ITreeItem *parentItem = static_cast<ITreeItem *>(parentIndex.internalPointer());
    QModelIndex modelIndex;
    if (parentItem && parentItem->isCategory()) {
        /* ボーン名または頂点モーフ名を含むアイテム */
        modelIndex = index(rowIndex, toModelIndex(frameIndex), parentIndex);
    }
    /* モデルのインデックスが存在しなければ作成しておき、自動的にそのインデックスが存在するように処理する */
    if (!modelIndex.isValid())
        createIndex(rowIndex, frameIndex, item);
    return modelIndex;
}

int PMDMotionModel::columnCount(const QModelIndex & /* parent */) const
{
    /* カラムは常に1つ以上存在するようにしないと assertion error が発生する */
    return m_modelRef ? maxFrameCount() + 1 : 1;
}

void PMDMotionModel::markAsNew(IModel *model)
{
    if (model == m_modelRef)
        setModified(false);
}

void PMDMotionModel::updateModel(IModel *model, bool seek)
{
    m_sceneRef->updateModel(model ? model : m_modelRef);
    if (seek)
        emit timeIndexDidChange(m_timeIndex, m_timeIndex);
}

void PMDMotionModel::refreshModel(IModel *model)
{
    if (model) {
        /* モデルのフレーム移動なしの更新とテーブルモデルの更新両方を含む */
        updateModel(model, true);
        setFrameIndexColumnMax(0);
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
        beginResetModel();
        endResetModel();
#else
        reset();
#endif
        emit motionDidUpdate(model);
    }
}

void PMDMotionModel::setActiveUndoStack()
{
    if (m_stacks.contains(m_modelRef))
        m_undoRef->setActiveStack(m_stacks[m_modelRef].data());
    else
        m_undoRef->setActiveStack(0);
}

int PMDMotionModel::maxFrameIndex() const
{
    return m_motionRef ? m_motionRef->maxTimeIndex() : 0;
}

bool PMDMotionModel::forceCameraUpdate() const
{
    return false;
}

void PMDMotionModel::setSceneRef(const Scene *value)
{
    m_sceneRef = value;
}

void PMDMotionModel::addPMDModel(IModel *model, const RootPtr &root, const Keys &keys)
{
    /* モデルが新規の場合はそのモデルの巻き戻しスタックを作成し、そうでない場合は該当のモデルの巻戻しスタックを有効にする */
    if (!m_stacks.contains(model)) {
        UndoStackPtr stackPtr(new QUndoStack());
        QUndoStack *stack = stackPtr.data();
        m_stacks.insert(model, stackPtr);
        m_undoRef->addStack(stack);
        m_undoRef->setActiveStack(stack);
    }
    else {
        m_undoRef->setActiveStack(m_stacks[model].data());
    }
    /* 各モデル毎のルートアイテム、ボーンまたは頂点モーフのキー名、テーブルのデータを作成する。作成済みの場合は何も処理しない */
    if (!m_roots.contains(model))
        m_roots.insert(model, root);
    if (!m_keys.contains(model))
        m_keys.insert(model, keys);
    if (!m_values.contains(model))
        m_values.insert(model, Values());
    /* 最初のキーフレーム登録が正しく行われるようにするため更新しておく必要がある */
    setFrameIndexColumnMax(0);
}

void PMDMotionModel::removePMDModel(IModel *model)
{
    qDebug("Removing a model from PMDMotionModel: %s", qPrintable(toQStringFromModel(m_modelRef)));
    /* PMD 追加で作成されたテーブルのモデルのデータと巻き戻しスタックの破棄を行う。モデルは削除されない */
    m_modelRef = 0;
    /* モーションのポインタを残すとダングリングポインタと化してクラッシュするので、ゼロクリアする */
    m_motionRef = 0;
    m_undoRef->setActiveStack(0);
    m_values.remove(model);
    m_keys.remove(model);
    m_stacks.remove(model);
    m_roots.remove(model);
    setFrameIndexColumnMax(0);
}

void PMDMotionModel::removePMDMotion(IModel *model)
{
    qDebug("Removing a motion from PMDMotionModel: %s", qPrintable(toQStringFromMotion(m_motionRef)));
    /* テーブルのモデルのデータの破棄と巻き戻しスタックの破棄を行う。モーションは削除されない */
    if (m_values.contains(model))
        m_values[model].clear();
    QUndoStack *stack = m_undoRef->activeStack();
    if (stack)
        stack->clear();
}

} /* namespace vpvm */
