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

#ifndef VPVM_GRAVITYSETTINGDIALOG_H
#define VPVM_GRAVITYSETTINGDIALOG_H

#include <QtGlobal>
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#include <QtWidgets/QDialog>
#else
#include <QtGui/QDialog>
#endif
#include <vpvl2/Common.h>

class QDoubleSpinBox;
class QGroupBox;
class QLabel;
class QSpinBox;

namespace vpvm
{

using namespace vpvl2;
class SceneLoader;

class GravitySettingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GravitySettingDialog(SceneLoader *loader, QWidget *parent = 0);
    ~GravitySettingDialog();

signals:
    void worldGravityDidSet(const Vector3 &value);
    void worldRandSeedDidSet(unsigned long value);

private slots:
    void retranslate();
    void emitSignal();

private:
    QDoubleSpinBox *createSpinBox(double value) const;

    QScopedPointer<QGroupBox> m_axisGroup;
    QScopedPointer<QLabel> m_axisXLabel;
    QScopedPointer<QDoubleSpinBox> m_axisX;
    QScopedPointer<QLabel> m_axisYLabel;
    QScopedPointer<QDoubleSpinBox> m_axisY;
    QScopedPointer<QLabel> m_axisZLabel;
    QScopedPointer<QDoubleSpinBox> m_axisZ;
    QScopedPointer<QLabel> m_randSeedLabel;
    QScopedPointer<QSpinBox> m_randSeedSpinBox;
};

} /* namespace vpvm */

#endif // GRAVITYSETTINGDIALOG_H
