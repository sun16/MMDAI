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

#include "vpvl2/vpvl2.h"
#include "vpvl2/internal/util.h"

#include "vpvl2/mvd/AssetSection.h"
#include "vpvl2/mvd/BoneSection.h"
#include "vpvl2/mvd/CameraSection.h"
#include "vpvl2/mvd/EffectSection.h"
#include "vpvl2/mvd/LightSection.h"
#include "vpvl2/mvd/ModelSection.h"
#include "vpvl2/mvd/MorphSection.h"
#include "vpvl2/mvd/Motion.h"
#include "vpvl2/mvd/NameListSection.h"
#include "vpvl2/mvd/ProjectSection.h"

namespace vpvl2
{
namespace mvd
{

const uint8_t *Motion::kSignature = reinterpret_cast<const uint8_t *>("Motion Vector Data file");

#pragma pack(push, 1)

struct Header {
    uint8_t signature[30];
    float version;
    uint8_t encoding;
};

#pragma pack(pop)

const QuadWord Motion::InterpolationTable::kDefaultParameter = QuadWord(20, 20, 107, 107);

Motion::InterpolationTable::InterpolationTable()
    : parameter(kDefaultParameter),
      linear(true),
      size(0)
{
}

Motion::InterpolationTable::~InterpolationTable()
{
    parameter = kDefaultParameter;
    linear = true;
    size = 0;
}

const QuadWord Motion::InterpolationTable::toQuadWord(const InterpolationPair &pair)
{
    return QuadWord(pair.first.x, pair.first.y, pair.second.x, pair.second.y);
}

void Motion::InterpolationTable::getInterpolationPair(InterpolationPair &pair) const
{
    pair.first.x = uint8_t(parameter.x());
    pair.first.y = uint8_t(parameter.y());
    pair.second.x = uint8_t(parameter.z());
    pair.second.y = uint8_t(parameter.w());
}

void Motion::InterpolationTable::build(const QuadWord &value, int s)
{
    if (!btFuzzyZero(value.x() - value.y()) || !btFuzzyZero(value.z() - value.w())) {
        table.resize(s + 1);
        const IKeyframe::SmoothPrecision &x1 = value.x() / 127.0, &x2 = value.z() / 127.0;
        const IKeyframe::SmoothPrecision &y1 = value.y() / 127.0, &y2 = value.w() / 127.0;
        IKeyframe::SmoothPrecision *ptr = &table[0];
        internal::buildInterpolationTable(x1, x2, y1, y2, s, ptr);
        linear = false;
    }
    else {
        table.clear();
        linear = true;
    }
    parameter = value;
    size = s;
}

void Motion::InterpolationTable::reset()
{
    table.clear();
    linear = true;
    parameter = kDefaultParameter;
}

//
// implemented
// - Bone
// - Camera
// - Morph
//
// NOT implemented
// - Asset
// - Effect
// - Light
// - Model
// - Project

Motion::Motion(IModel *modelRef, IEncoding *encodingRef)
    : m_motionPtr(0),
      m_assetSection(0),
      m_boneSection(0),
      m_cameraSection(0),
      m_effectSection(0),
      m_lightSection(0),
      m_modelSection(0),
      m_morphSection(0),
      m_nameListSection(0),
      m_projectSection(0),
      m_modelRef(modelRef),
      m_encodingRef(encodingRef),
      m_name(0),
      m_name2(0),
      m_reserved(0),
      m_error(kNoError),
      m_active(true)
{
    m_nameListSection = new NameListSection(encodingRef);
    m_assetSection = new AssetSection(m_nameListSection);
    m_boneSection = new BoneSection(modelRef, m_nameListSection);
    m_cameraSection = new CameraSection(m_nameListSection);
    m_effectSection = new EffectSection(m_nameListSection);
    m_lightSection = new LightSection(m_nameListSection);
    m_modelSection = new ModelSection(modelRef, m_nameListSection, 0);
    m_morphSection = new MorphSection(modelRef, m_nameListSection);
    m_projectSection = new ProjectSection(m_nameListSection);
}

Motion::~Motion()
{
    release();
}

bool Motion::preparse(const uint8_t *data, size_t size, DataInfo &info)
{
    size_t rest = size;
    // Header(30)
    Header header;
    if (sizeof(header) > rest) {
        m_error = kInvalidHeaderError;
        return false;
    }

    uint8_t *ptr = const_cast<uint8_t *>(data);
    info.basePtr = ptr;

    // Check the signature is valid
    internal::getData(ptr, header);
    if (memcmp(header.signature, kSignature, sizeof(kSignature) - 1) != 0) {
        m_error = kInvalidSignatureError;
        return false;
    }
    if (header.version != 1.0) {
        m_error = kInvalidVersionError;
        return false;
    }
    if (header.encoding != 0 && header.encoding != 1) {
        m_error = kInvalidEncodingError;
        return false;
    }
    info.codec = header.encoding == 0 ? IString::kUTF16 : IString::kUTF8;
    ptr += sizeof(header);
    rest -= sizeof(header);

    /* object name */
    if (!internal::sizeText(ptr, rest, info.namePtr, info.nameSize)) {
        return false;
    }
    /* object name2 */
    if (!internal::sizeText(ptr, rest, info.name2Ptr, info.name2Size)) {
        return false;
    }
    /* scene FPS */
    if (!internal::validateSize(ptr, sizeof(float), rest)) {
        return false;
    }
    /* reserved */
    if (!internal::sizeText(ptr, rest, info.reservedPtr, info.reservedSize)) {
        return false;
    }
    info.sectionStartPtr = ptr;

    /* sections */
    bool ret = false;
    while (rest > 0) {
        const SectionTag &sectionHeader = *reinterpret_cast<const SectionTag *>(ptr);
        if (!internal::validateSize(ptr, sizeof(sectionHeader), rest)) {
            return false;
        }
        uint8_t *startPtr = ptr;
        switch (static_cast<SectionType>(sectionHeader.type)) {
        case kNameListSection: {
            if (!NameListSection::preparse(ptr, rest, info)) {
                return false;
            }
            info.nameListSectionPtr = startPtr;
            break;
        }
        case kBoneSection: {
            if (!BoneSection::preparse(ptr, rest, info)) {
                return false;
            }
            info.boneSectionPtrs.add(startPtr);
            break;
        }
        case kMorphSection: {
            if (!MorphSection::preparse(ptr, rest, info)) {
                return false;
            }
            info.morphSectionPtrs.add(startPtr);
            break;
        }
        case kModelSection: {
            info.adjustAlignment = sectionHeader.minor == 1 ? 4 : 0;
            if (!ModelSection::preparse(ptr, rest, info)) {
                return false;
            }
            info.modelSectionPtrs.add(startPtr);
            break;
        }
        case kAssetSection: {
            if (!AssetSection::preparse(ptr, rest, info)) {
                return false;
            }
            info.assetSectionPtrs.add(startPtr);
            break;
        }
        case kEffectSection: {
            if (!EffectSection::preparse(ptr, rest, info)) {
                return false;
            }
            info.effectSectionPtrs.add(startPtr);
            break;
        }
        case kCameraSection: {
            if (!CameraSection::preparse(ptr, rest, info)) {
                return false;
            }
            info.cameraSectionPtrs.add(startPtr);
            break;
        }
        case kLightSection: {
            if (!LightSection::preparse(ptr, rest, info)) {
                return false;
            }
            info.lightSectionPtrs.add(startPtr);
            break;
        }
        case kProjectSection: {
            if (!ProjectSection::preparse(ptr, rest, info)) {
                return false;
            }
            info.projectSectionPtrs.add(startPtr);
            break;
        }
        case kEndOfFile: {
            ret = true;
            rest = 0;
            info.encoding = m_encodingRef;
            info.endPtr = ptr;
            break;
        }
        default:
            rest = 0;
            info.endPtr = 0;
            break;
        }
    }
    return ret;
}

bool Motion::load(const uint8_t *data, size_t size)
{
    DataInfo info;
    internal::zerofill(&info, sizeof(info));
    if (preparse(data, size, info)) {
        release();
        parseHeader(info);
        parseAssetSections(info);
        parseBoneSections(info);
        parseCameraSections(info);
        parseEffectSections(info);
        parseLightSections(info);
        parseModelSections(info);
        parseMorphSections(info);
        parseProjectSections(info);
        m_info.copy(info);
        return true;
    }
    return false;
}

void Motion::save(uint8_t *data) const
{
    Header header;
    IString::Codec codec = m_info.codec;
    internal::zerofill(&header, sizeof(header));
    memcpy(header.signature, kSignature, sizeof(header.signature) - 1);
    header.version = 1.0;
    header.encoding = 1;
    internal::writeBytes(reinterpret_cast<const uint8_t *>(&header), sizeof(header), data);
    internal::writeString(m_name, codec, data);
    internal::writeString(m_name2, codec, data);
    float fps = 30.0;
    internal::writeBytes(reinterpret_cast<const uint8_t *>(&fps), sizeof(fps), data);
    internal::writeString(m_reserved, codec, data);
    m_nameListSection->write(data, m_info);
    data += m_nameListSection->estimateSize(m_info);
    m_boneSection->write(data);
    data += m_boneSection->estimateSize();
    m_morphSection->write(data);
    data += m_morphSection->estimateSize();
    m_modelSection->write(data);
    data += m_modelSection->estimateSize();
    m_assetSection->write(data);
    data += m_assetSection->estimateSize();
    m_effectSection->write(data);
    data += m_effectSection->estimateSize();
    m_cameraSection->write(data);
    data += m_cameraSection->estimateSize();
    m_lightSection->write(data);
    data += m_lightSection->estimateSize();
    m_projectSection->write(data);
    data += m_projectSection->estimateSize();
    SectionTag eof;
    eof.type = kEndOfFile;
    eof.minor = 0;
    internal::writeBytes(reinterpret_cast<const uint8_t *>(&eof), sizeof(eof), data);
}

size_t Motion::estimateSize() const
{
    size_t size = 0;
    IString::Codec codec = m_info.codec;
    size += sizeof(Header);
    size += internal::estimateSize(m_name, codec);
    size += internal::estimateSize(m_name2, codec);
    size += sizeof(float);
    size += internal::estimateSize(m_reserved, codec);
    size += m_nameListSection->estimateSize(m_info);
    size += m_boneSection->estimateSize();
    size += m_morphSection->estimateSize();
    size += m_modelSection->estimateSize();
    size += m_assetSection->estimateSize();
    size += m_effectSection->estimateSize();
    size += m_cameraSection->estimateSize();
    size += m_lightSection->estimateSize();
    size += m_projectSection->estimateSize();
    size += sizeof(SectionTag);
    return size;
}

void Motion::setParentModel(IModel *model)
{
    m_modelRef = model;
    m_boneSection->setParentModel(model);
    m_modelSection->setParentModel(model);
    m_morphSection->setParentModel(model);
}

void Motion::seek(const IKeyframe::TimeIndex &timeIndex)
{
    m_assetSection->seek(timeIndex);
    m_boneSection->seek(timeIndex);
    m_modelSection->seek(timeIndex);
    m_morphSection->seek(timeIndex);
    m_active = maxTimeIndex() > timeIndex;
}

void Motion::seekScene(const IKeyframe::TimeIndex &timeIndex, Scene *scene)
{
    if (m_cameraSection->countKeyframes() > 0) {
        m_cameraSection->seek(timeIndex);
        ICamera *camera = scene->camera();
        camera->setLookAt(m_cameraSection->position());
        camera->setAngle(m_cameraSection->angle());
        camera->setFov(m_cameraSection->fov());
        camera->setDistance(m_cameraSection->distance());
    }
}

void Motion::advance(const IKeyframe::TimeIndex &deltaTimeIndex)
{
    m_assetSection->advance(deltaTimeIndex);
    m_boneSection->advance(deltaTimeIndex);
    m_effectSection->advance(deltaTimeIndex);
    m_modelSection->advance(deltaTimeIndex);
    m_morphSection->advance(deltaTimeIndex);
    if (deltaTimeIndex > 0)
        m_active = !isReachedTo(maxTimeIndex());
}

void Motion::advanceScene(const IKeyframe::TimeIndex &deltaTimeIndex, Scene *scene)
{
    if (m_cameraSection->countKeyframes() > 0) {
        m_cameraSection->advance(deltaTimeIndex);
        ICamera *camera = scene->camera();
        camera->setLookAt(m_cameraSection->position());
        camera->setAngle(m_cameraSection->angle());
        camera->setFov(m_cameraSection->fov());
        camera->setDistance(m_cameraSection->distance());
    }
}

void Motion::reload()
{
}

void Motion::reset()
{
    m_assetSection->seek(0);
    m_boneSection->seek(0);
    m_cameraSection->seek(0);
    m_effectSection->seek(0);
    m_lightSection->seek(0);
    m_modelSection->seek(0);
    m_morphSection->seek(0);
    m_projectSection->seek(0);
    m_active = true;
}

const IKeyframe::TimeIndex &Motion::maxTimeIndex() const
{
    return btMax(m_assetSection->maxTimeIndex(),
                 btMax(m_boneSection->maxTimeIndex(),
                       btMax(m_cameraSection->maxTimeIndex(),
                             btMax(m_effectSection->maxTimeIndex(),
                                   btMax(m_lightSection->maxTimeIndex(),
                                         btMax(m_modelSection->maxTimeIndex(),
                                               btMax(m_morphSection->maxTimeIndex(),
                                                     m_projectSection->maxTimeIndex())))))));
}

bool Motion::isReachedTo(const IKeyframe::TimeIndex &atEnd) const
{
    return !m_active  || (m_assetSection->currentTimeIndex() >= atEnd &&
                          m_boneSection->currentTimeIndex() >= atEnd &&
                          m_effectSection->currentTimeIndex() >= atEnd &&
                          m_modelSection->currentTimeIndex() >= atEnd &&
                          m_morphSection->currentTimeIndex() >= atEnd);
}

bool Motion::isNullFrameEnabled() const
{
    return false;
}

void Motion::setNullFrameEnable(bool /* value */)
{
}

void Motion::addKeyframe(IKeyframe *value)
{
    if (!value)
        return;
    switch (value->type()) {
    case IKeyframe::kAsset:
        m_assetSection->addKeyframe(value);
        break;
    case IKeyframe::kBone:
        m_boneSection->addKeyframe(value);
        break;
    case IKeyframe::kCamera:
        m_cameraSection->addKeyframe(value);
        break;
    case IKeyframe::kEffect:
        m_effectSection->addKeyframe(value);
        break;
    case IKeyframe::kLight:
        m_lightSection->addKeyframe(value);
        break;
    case IKeyframe::kModel:
        m_modelSection->addKeyframe(value);
        break;
    case IKeyframe::kMorph:
        m_morphSection->addKeyframe(value);
        break;
    case IKeyframe::kProject:
        m_projectSection->addKeyframe(value);
        break;
    default:
        break;
    }
}

void Motion::replaceKeyframe(IKeyframe *value)
{
    if (!value)
        return;
    switch (value->type()) {
    case IKeyframe::kAsset: {
        break;
    }
    case IKeyframe::kBone: {
        IKeyframe *prev = m_boneSection->findKeyframe(value->timeIndex(), value->name(), value->layerIndex());
        m_boneSection->deleteKeyframe(prev);
        m_boneSection->addKeyframe(value);
        break;
    }
    case IKeyframe::kCamera: {
        IKeyframe *prev = m_cameraSection->findKeyframe(value->timeIndex(), value->layerIndex());
        m_cameraSection->deleteKeyframe(prev);
        m_cameraSection->addKeyframe(value);
        break;
    }
    case IKeyframe::kEffect: {
        break;
    }
    case IKeyframe::kLight: {
        break;
    }
    case IKeyframe::kModel: {
        break;
    }
    case IKeyframe::kMorph: {
        IKeyframe *prev = m_morphSection->findKeyframe(value->timeIndex(), value->name(), value->layerIndex());
        m_morphSection->deleteKeyframe(prev);
        m_morphSection->addKeyframe(value);
        break;
    }
    case IKeyframe::kProject: {
        break;
    }
    default:
        break;
    }
}

int Motion::countKeyframes(IKeyframe::Type value) const
{
    switch (value) {
    case IKeyframe::kAsset:
        return m_assetSection->countKeyframes();
    case IKeyframe::kBone:
        return m_boneSection->countKeyframes();
    case IKeyframe::kCamera:
        return m_cameraSection->countKeyframes();
    case IKeyframe::kEffect:
        return m_effectSection->countKeyframes();
    case IKeyframe::kLight:
        return m_lightSection->countKeyframes();
    case IKeyframe::kModel:
        return m_modelSection->countKeyframes();
    case IKeyframe::kMorph:
        return m_morphSection->countKeyframes();
    case IKeyframe::kProject:
        return m_projectSection->countKeyframes();
    default:
        return 0;
    }
}

IKeyframe::LayerIndex Motion::countLayers(const IString *name,
                                          IKeyframe::Type type) const
{
    switch (type) {
    case IKeyframe::kBone:
        return m_boneSection->countLayers(name);
    case IKeyframe::kCamera:
        return m_cameraSection->countLayers();
    default:
        return 1;
    }
}

void Motion::getKeyframes(const IKeyframe::TimeIndex &timeIndex,
                          const IKeyframe::LayerIndex &layerIndex,
                          IKeyframe::Type type,
                          Array<IKeyframe *> &keyframes)
{
    switch (type) {
    case IKeyframe::kAsset:
        m_assetSection->getKeyframes(timeIndex, layerIndex, keyframes);
        break;
    case IKeyframe::kBone:
        m_boneSection->getKeyframes(timeIndex, layerIndex, keyframes);
        break;
    case IKeyframe::kCamera:
        m_cameraSection->getKeyframes(timeIndex, layerIndex, keyframes);
        break;
    case IKeyframe::kEffect:
        m_effectSection->getKeyframes(timeIndex, layerIndex, keyframes);
        break;
    case IKeyframe::kLight:
        m_lightSection->getKeyframes(timeIndex, layerIndex, keyframes);
        break;
    case IKeyframe::kModel:
        m_modelSection->getKeyframes(timeIndex, layerIndex, keyframes);
        break;
    case IKeyframe::kMorph:
        m_morphSection->getKeyframes(timeIndex, layerIndex, keyframes);
        break;
    case IKeyframe::kProject:
        m_projectSection->getKeyframes(timeIndex, layerIndex, keyframes);
        break;
    default:
        break;
    }
}

IBoneKeyframe *Motion::findBoneKeyframe(const IKeyframe::TimeIndex &timeIndex,
                                        const IString *name,
                                        const IKeyframe::LayerIndex &layerIndex) const
{
    return m_boneSection->findKeyframe(timeIndex, name, layerIndex);
}

IBoneKeyframe *Motion::findBoneKeyframeAt(int index) const
{
    return m_boneSection->findKeyframeAt(index);
}

ICameraKeyframe *Motion::findCameraKeyframe(const IKeyframe::TimeIndex &timeIndex,
                                            const IKeyframe::LayerIndex &layerIndex) const
{
    return m_cameraSection->findKeyframe(timeIndex, layerIndex);
}

ICameraKeyframe *Motion::findCameraKeyframeAt(int index) const
{
    return m_cameraSection->findKeyframeAt(index);
}

ILightKeyframe *Motion::findLightKeyframe(const IKeyframe::TimeIndex &timeIndex,
                                          const IKeyframe::LayerIndex &layerIndex) const
{
    return m_lightSection->findKeyframe(timeIndex, layerIndex);
}

ILightKeyframe *Motion::findLightKeyframeAt(int index) const
{
    return m_lightSection->findKeyframeAt(index);
}

IMorphKeyframe *Motion::findMorphKeyframe(const IKeyframe::TimeIndex &timeIndex,
                                          const IString *name,
                                          const IKeyframe::LayerIndex &layerIndex) const
{
    return m_morphSection->findKeyframe(timeIndex, name, layerIndex);
}

IMorphKeyframe *Motion::findMorphKeyframeAt(int index) const
{
    return m_morphSection->findKeyframeAt(index);
}

void Motion::deleteKeyframe(IKeyframe *&value)
{
    /* prevent deleting a null keyframe and timeIndex() of the keyframe is zero */
    if (!value || value->timeIndex() == 0)
        return;
    switch (value->type()) {
    case IKeyframe::kAsset:
        m_assetSection->deleteKeyframe(value);
        break;
    case IKeyframe::kBone:
        m_boneSection->deleteKeyframe(value);
        break;
    case IKeyframe::kCamera:
        m_cameraSection->deleteKeyframe(value);
        break;
    case IKeyframe::kEffect:
        m_effectSection->deleteKeyframe(value);
        break;
    case IKeyframe::kLight:
        m_lightSection->deleteKeyframe(value);
        break;
    case IKeyframe::kModel:
        m_modelSection->deleteKeyframe(value);
        break;
    case IKeyframe::kMorph:
        m_morphSection->deleteKeyframe(value);
        break;
    case IKeyframe::kProject:
        m_projectSection->deleteKeyframe(value);
        break;
    default:
        break;
    }
}

void Motion::update(IKeyframe::Type type)
{
    switch (type) {
    case IKeyframe::kAsset:
        break;
    case IKeyframe::kBone:
        break;
    case IKeyframe::kCamera:
        break;
    case IKeyframe::kEffect:
        break;
    case IKeyframe::kLight:
        break;
    case IKeyframe::kModel:
        break;
    case IKeyframe::kMorph:
        break;
    case IKeyframe::kProject:
        break;
    default:
        break;
    }
}

IMotion *Motion::clone() const
{
    IMotion *motion = m_motionPtr = new Motion(m_modelRef, m_encodingRef);
    int nBoneKeyframes = countKeyframes(IKeyframe::kBone);
    for (int i = 0; i < nBoneKeyframes; i++) {
        IBoneKeyframe *keyframe = findBoneKeyframeAt(i);
        motion->addKeyframe(keyframe->clone());
    }
    int nCameraKeyframes = countKeyframes(IKeyframe::kCamera);
    for (int i = 0; i < nCameraKeyframes; i++) {
        ICameraKeyframe *keyframe = findCameraKeyframeAt(i);
        motion->addKeyframe(keyframe->clone());
    }
    int nLightKeyframe = countKeyframes(IKeyframe::kLight);
    for (int i = 0; i < nLightKeyframe; i++) {
        ILightKeyframe *keyframe = findLightKeyframeAt(i);
        motion->addKeyframe(keyframe->clone());
    }
    int nMorphKeyframes = countKeyframes(IKeyframe::kMorph);
    for (int i = 0; i < nMorphKeyframes; i++) {
        IMorphKeyframe *keyframe = findMorphKeyframeAt(i);
        motion->addKeyframe(keyframe->clone());
    }
    m_motionPtr = 0;
    return motion;
}

void Motion::parseHeader(const DataInfo &info)
{
    IEncoding *encoding = info.encoding;
    internal::setStringDirect(encoding->toString(info.namePtr, info.nameSize, info.codec), m_name);
    internal::setStringDirect(encoding->toString(info.name2Ptr, info.name2Size, info.codec), m_name2);
    internal::setStringDirect(encoding->toString(info.reservedPtr, info.reservedSize, info.codec), m_reserved);
    m_nameListSection = new NameListSection(m_encodingRef);
    m_nameListSection->read(info.nameListSectionPtr, info.codec);
}

void Motion::parseAssetSections(const DataInfo &info)
{
    const Array<uint8_t *> &sections = info.assetSectionPtrs;
    const int nsections = sections.count();
    m_assetSection = new AssetSection(m_nameListSection);
    for (int i = 0; i < nsections; i++) {
        const uint8_t *ptr = sections[i];
        m_assetSection->read(ptr);
    }
}

void Motion::parseBoneSections(const DataInfo &info)
{
    const Array<uint8_t *> &sections = info.boneSectionPtrs;
    const int nsections = sections.count();
    m_boneSection = new BoneSection(m_modelRef, m_nameListSection);
    for (int i = 0; i < nsections; i++) {
        const uint8_t *ptr = sections[i];
        m_boneSection->read(ptr);
    }
}

void Motion::parseCameraSections(const DataInfo &info)
{
    const Array<uint8_t *> &sections = info.cameraSectionPtrs;
    const int nsections = sections.count();
    m_cameraSection = new CameraSection(m_nameListSection);
    for (int i = 0; i < nsections; i++) {
        const uint8_t *ptr = sections[i];
        m_cameraSection->read(ptr);
    }
}

void Motion::parseEffectSections(const DataInfo &info)
{
    const Array<uint8_t *> &sections = info.effectSectionPtrs;
    const int nsections = sections.count();
    m_effectSection = new EffectSection(m_nameListSection);
    for (int i = 0; i < nsections; i++) {
        const uint8_t *ptr = sections[i];
        m_effectSection->read(ptr);
    }
}

void Motion::parseLightSections(const DataInfo &info)
{
    const Array<uint8_t *> &sections = info.lightSectionPtrs;
    const int nsections = sections.count();
    m_lightSection = new LightSection(m_nameListSection);
    for (int i = 0; i < nsections; i++) {
        const uint8_t *ptr = sections[i];
        m_lightSection->read(ptr);
    }
}

void Motion::parseModelSections(const DataInfo &info)
{
    const Array<uint8_t *> &sections = info.modelSectionPtrs;
    const int nsections = sections.count();
    m_modelSection = new ModelSection(m_modelRef, m_nameListSection, info.adjustAlignment);
    for (int i = 0; i < nsections; i++) {
        const uint8_t *ptr = sections[i];
        m_modelSection->read(ptr);
    }
}

void Motion::parseMorphSections(const DataInfo &info)
{
    const Array<uint8_t *> &sections = info.morphSectionPtrs;
    const int nsections = sections.count();
    m_morphSection = new MorphSection(m_modelRef, m_nameListSection);
    for (int i = 0; i < nsections; i++) {
        const uint8_t *ptr = sections[i];
        m_morphSection->read(ptr);
    }
}

void Motion::parseProjectSections(const DataInfo &info)
{
    const Array<uint8_t *> &sections = info.projectSectionPtrs;
    const int nsections = sections.count();
    m_projectSection = new ProjectSection(m_nameListSection);
    for (int i = 0; i < nsections; i++) {
        const uint8_t *ptr = sections[i];
        m_projectSection->read(ptr);
    }
}

void Motion::release()
{
    delete m_motionPtr;
    m_motionPtr = 0;
    delete m_assetSection;
    m_assetSection = 0;
    delete m_boneSection;
    m_boneSection = 0;
    delete m_cameraSection;
    m_cameraSection = 0;
    delete m_effectSection;
    m_effectSection = 0;
    delete m_lightSection;
    m_lightSection = 0;
    delete m_modelSection;
    m_modelSection = 0;
    delete m_morphSection;
    m_morphSection = 0;
    delete m_nameListSection;
    m_nameListSection = 0;
    delete m_projectSection;
    m_projectSection = 0;
    delete m_name;
    m_name = 0;
    delete m_name2;
    m_name2 = 0;
    delete m_reserved;
    m_reserved = 0;
    m_error = kNoError;
    m_active = false;
}

} /* namespace mvd */
} /* namespace vpvl2 */

