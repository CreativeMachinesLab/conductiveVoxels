/*******************************************************************************
Copyright (c) 2010, Jonathan Hiller
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * The name if its contributors may not be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************/

#include "Dlg_Tensile.h"
#include "QVX_Interfaces.h"


Dlg_Tensile::Dlg_Tensile(QVX_Environment* pEnvIn, QWidget *parent)
	: QWidget(parent)
{
	pEnv = pEnvIn;
	ui.setupUi(this);

	ui.NumStepSpin->setValue(10);
	ui.ConvThreshEdit->setText(QString::number(1e-7));
	ui.MixRadiusEdit->setText(QString::number(0.0));
	ui.PolyExpEdit->setText(QString::number(2.0));

	
	ui.mm_LinearButton->setChecked(true);

	const QValidator* DEval = new QDoubleValidator(this);
	ui.ConvThreshEdit->setValidator(DEval);
	ui.MixRadiusEdit->setValidator(DEval);
	ui.PolyExpEdit->setValidator(DEval);

	connect(ui.DoneButton, SIGNAL(clicked()), this, SIGNAL(DoneTensileTesting()));
	connect(ui.StartButton, SIGNAL(clicked()), this, SLOT(StartTest()));

}

Dlg_Tensile::~Dlg_Tensile()
{

}

void Dlg_Tensile::StartTest(void)
{
	MatBlendModel BlendModelIn;
	if (ui.mm_LinearButton->isChecked()) BlendModelIn=MB_LINEAR;
	else if (ui.mm_ExpButton->isChecked()) BlendModelIn=MB_EXPONENTIAL;
	else if (ui.mm_PolyButton->isChecked()) BlendModelIn=MB_POLYNOMIAL;

	Tester.BeginTensileTest(pEnv, ui.NumStepSpin->value(), ui.ConvThreshEdit->text().toDouble(), ui.MixRadiusEdit->text().toDouble(), BlendModelIn, ui.PolyExpEdit->text().toDouble());
}
