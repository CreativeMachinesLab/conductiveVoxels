/********************************************************************************
** Form generated from reading UI file 'vTensile.ui'
**
** Created by: Qt User Interface Compiler version 4.8.6
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_VTENSILE_H
#define UI_VTENSILE_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QGroupBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>
#include <QtGui/QRadioButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QSpinBox>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_TensileDlg
{
public:
    QVBoxLayout *verticalLayout_4;
    QHBoxLayout *horizontalLayout_3;
    QSpinBox *NumStepSpin;
    QLabel *label;
    QHBoxLayout *horizontalLayout_4;
    QVBoxLayout *verticalLayout_2;
    QLineEdit *ConvThreshEdit;
    QLineEdit *MixRadiusEdit;
    QVBoxLayout *verticalLayout_3;
    QLabel *label_2;
    QLabel *label_3;
    QGroupBox *groupBox;
    QVBoxLayout *verticalLayout;
    QRadioButton *mm_LinearButton;
    QRadioButton *mm_ExpButton;
    QHBoxLayout *horizontalLayout_2;
    QRadioButton *mm_PolyButton;
    QSpacerItem *horizontalSpacer;
    QLineEdit *PolyExpEdit;
    QLabel *label_4;
    QSpacerItem *verticalSpacer;
    QHBoxLayout *horizontalLayout;
    QPushButton *StartButton;
    QPushButton *DoneButton;

    void setupUi(QWidget *TensileDlg)
    {
        if (TensileDlg->objectName().isEmpty())
            TensileDlg->setObjectName(QString::fromUtf8("TensileDlg"));
        TensileDlg->resize(242, 256);
        verticalLayout_4 = new QVBoxLayout(TensileDlg);
        verticalLayout_4->setObjectName(QString::fromUtf8("verticalLayout_4"));
        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        NumStepSpin = new QSpinBox(TensileDlg);
        NumStepSpin->setObjectName(QString::fromUtf8("NumStepSpin"));
        NumStepSpin->setMinimum(1);
        NumStepSpin->setMaximum(10000);
        NumStepSpin->setSingleStep(10);

        horizontalLayout_3->addWidget(NumStepSpin);

        label = new QLabel(TensileDlg);
        label->setObjectName(QString::fromUtf8("label"));

        horizontalLayout_3->addWidget(label);


        verticalLayout_4->addLayout(horizontalLayout_3);

        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        verticalLayout_2 = new QVBoxLayout();
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        ConvThreshEdit = new QLineEdit(TensileDlg);
        ConvThreshEdit->setObjectName(QString::fromUtf8("ConvThreshEdit"));

        verticalLayout_2->addWidget(ConvThreshEdit);

        MixRadiusEdit = new QLineEdit(TensileDlg);
        MixRadiusEdit->setObjectName(QString::fromUtf8("MixRadiusEdit"));

        verticalLayout_2->addWidget(MixRadiusEdit);


        horizontalLayout_4->addLayout(verticalLayout_2);

        verticalLayout_3 = new QVBoxLayout();
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        label_2 = new QLabel(TensileDlg);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        verticalLayout_3->addWidget(label_2);

        label_3 = new QLabel(TensileDlg);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        verticalLayout_3->addWidget(label_3);


        horizontalLayout_4->addLayout(verticalLayout_3);


        verticalLayout_4->addLayout(horizontalLayout_4);

        groupBox = new QGroupBox(TensileDlg);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        verticalLayout = new QVBoxLayout(groupBox);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        mm_LinearButton = new QRadioButton(groupBox);
        mm_LinearButton->setObjectName(QString::fromUtf8("mm_LinearButton"));

        verticalLayout->addWidget(mm_LinearButton);

        mm_ExpButton = new QRadioButton(groupBox);
        mm_ExpButton->setObjectName(QString::fromUtf8("mm_ExpButton"));

        verticalLayout->addWidget(mm_ExpButton);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        mm_PolyButton = new QRadioButton(groupBox);
        mm_PolyButton->setObjectName(QString::fromUtf8("mm_PolyButton"));

        horizontalLayout_2->addWidget(mm_PolyButton);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer);

        PolyExpEdit = new QLineEdit(groupBox);
        PolyExpEdit->setObjectName(QString::fromUtf8("PolyExpEdit"));
        QSizePolicy sizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(PolyExpEdit->sizePolicy().hasHeightForWidth());
        PolyExpEdit->setSizePolicy(sizePolicy);

        horizontalLayout_2->addWidget(PolyExpEdit);

        label_4 = new QLabel(groupBox);
        label_4->setObjectName(QString::fromUtf8("label_4"));

        horizontalLayout_2->addWidget(label_4);


        verticalLayout->addLayout(horizontalLayout_2);


        verticalLayout_4->addWidget(groupBox);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_4->addItem(verticalSpacer);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        StartButton = new QPushButton(TensileDlg);
        StartButton->setObjectName(QString::fromUtf8("StartButton"));

        horizontalLayout->addWidget(StartButton);

        DoneButton = new QPushButton(TensileDlg);
        DoneButton->setObjectName(QString::fromUtf8("DoneButton"));

        horizontalLayout->addWidget(DoneButton);


        verticalLayout_4->addLayout(horizontalLayout);


        retranslateUi(TensileDlg);

        QMetaObject::connectSlotsByName(TensileDlg);
    } // setupUi

    void retranslateUi(QWidget *TensileDlg)
    {
        TensileDlg->setWindowTitle(QApplication::translate("TensileDlg", "Tensile Test", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("TensileDlg", "Number of Steps", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("TensileDlg", "Converge Thresh (mm/timestep)", 0, QApplication::UnicodeUTF8));
        label_3->setText(QApplication::translate("TensileDlg", "Std Dev Mixing Radius (Voxels)", 0, QApplication::UnicodeUTF8));
        groupBox->setTitle(QApplication::translate("TensileDlg", "Material Mixing Model", 0, QApplication::UnicodeUTF8));
        mm_LinearButton->setText(QApplication::translate("TensileDlg", "Linear (x)", 0, QApplication::UnicodeUTF8));
        mm_ExpButton->setText(QApplication::translate("TensileDlg", "Exponential (2^x-1)", 0, QApplication::UnicodeUTF8));
        mm_PolyButton->setText(QApplication::translate("TensileDlg", "Polynomial (x^n)", 0, QApplication::UnicodeUTF8));
        label_4->setText(QApplication::translate("TensileDlg", "n", 0, QApplication::UnicodeUTF8));
        StartButton->setText(QApplication::translate("TensileDlg", "Start", 0, QApplication::UnicodeUTF8));
        DoneButton->setText(QApplication::translate("TensileDlg", "Done", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class TensileDlg: public Ui_TensileDlg {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_VTENSILE_H
