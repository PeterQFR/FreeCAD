/***************************************************************************
 *   Copyright (c) 2011 Juergen Riegel <FreeCAD@juergen-riegel.net>        *
 *                                                                         *
 *   This file is part of the FreeCAD CAx development system.              *
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Library General Public           *
 *   License as published by the Free Software Foundation; either          *
 *   version 2 of the License, or (at your option) any later version.      *
 *                                                                         *
 *   This library  is distributed in the hope that it will be useful,      *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU Library General Public License for more details.                  *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this library; see the file COPYING.LIB. If not,    *
 *   write to the Free Software Foundation, Inc., 59 Temple Place,         *
 *   Suite 330, Boston, MA  02111-1307, USA                                *
 *                                                                         *
 ***************************************************************************/


#include "PreCompiled.h"

#ifndef _PreComp_
# include <QAction>
#endif

#include "ui_TaskChamferParameters.h"
#include "TaskChamferParameters.h"
#include <App/Application.h>
#include <App/Document.h>
#include <Gui/Application.h>
#include <Gui/Document.h>
#include <Gui/BitmapFactory.h>
#include <Gui/ViewProvider.h>
#include <Gui/WaitCursor.h>
#include <Base/Console.h>
#include <Base/Tools.h>
#include <Base/UnitsApi.h>
#include <Gui/Selection.h>
#include <Gui/Command.h>
#include <Mod/PartDesign/App/FeatureChamfer.h>
#include <Mod/PartDesign/App/Body.h>
#include <Mod/Sketcher/App/SketchObject.h>


using namespace PartDesignGui;
using namespace Gui;

/* TRANSLATOR PartDesignGui::TaskChamferParameters */

TaskChamferParameters::TaskChamferParameters(ViewProviderDressUp *DressUpView,QWidget *parent)
    : TaskDressUpParameters(DressUpView, true, true, parent)
{
    // we need a separate container widget to add all controls to
    proxy = new QWidget(this);
    ui = new Ui_TaskChamferParameters();
    ui->setupUi(proxy);
    this->groupLayout()->addWidget(proxy);

    PartDesign::Chamfer* pcChamfer = static_cast<PartDesign::Chamfer*>(DressUpView->getObject());
    double r = pcChamfer->Size.getValue();

    ui->chamferDistance->setUnit(Base::Unit::Length);
    ui->chamferDistance->setValue(r);
    ui->chamferDistance->setMinimum(0);
    ui->chamferDistance->selectNumber();
    ui->chamferDistance->bind(pcChamfer->Size);
    QMetaObject::invokeMethod(ui->chamferDistance, "setFocus", Qt::QueuedConnection);

    QMetaObject::connectSlotsByName(this);

    connect(ui->chamferDistance, SIGNAL(valueChanged(double)),
            this, SLOT(onLengthChanged(double)));

    setup(ui->message, ui->listWidgetReferences, ui->buttonRefAdd);
}

void TaskChamferParameters::refresh()
{
    if(!DressUpView)
        return;

    TaskDressUpParameters::refresh();

    PartDesign::Chamfer* pcChamfer = static_cast<PartDesign::Chamfer*>(DressUpView->getObject());
    double r = pcChamfer->Size.getValue();

    {
        QSignalBlocker blocker(ui->chamferDistance);
        ui->chamferDistance->setValue(r);
    }
}

void TaskChamferParameters::onLengthChanged(double len)
{
    if(!DressUpView)
        return;

    PartDesign::Chamfer* pcChamfer = static_cast<PartDesign::Chamfer*>(DressUpView->getObject());
    setupTransaction();
    pcChamfer->Size.setValue(len);
    recompute();
}

double TaskChamferParameters::getLength(void) const
{
    return ui->chamferDistance->value().getValue();
}

TaskChamferParameters::~TaskChamferParameters()
{
    Gui::Selection().rmvSelectionGate();
    delete ui;
}

void TaskChamferParameters::changeEvent(QEvent *e)
{
    TaskBox::changeEvent(e);
    if (e->type() == QEvent::LanguageChange) {
        ui->retranslateUi(proxy);
    }
}

void TaskChamferParameters::apply()
{
    if(!DressUpView)
        return;

    std::string name = DressUpView->getObject()->getNameInDocument();

    //Gui::Command::openCommand("Chamfer changed");
    ui->chamferDistance->apply();
}

//**************************************************************************
//**************************************************************************
// TaskDialog
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

TaskDlgChamferParameters::TaskDlgChamferParameters(ViewProviderChamfer *DressUpView)
    : TaskDlgDressUpParameters(DressUpView)
{
    parameter  = new TaskChamferParameters(DressUpView);

    Content.push_back(parameter);
}

TaskDlgChamferParameters::~TaskDlgChamferParameters()
{

}

//==== calls from the TaskView ===============================================================


//void TaskDlgChamferParameters::open()
//{
//    // a transaction is already open at creation time of the chamfer
//    if (!Gui::Command::hasPendingCommand()) {
//        QString msg = tr("Edit chamfer");
//        Gui::Command::openCommand((const char*)msg.toUtf8());
//    }
//}
bool TaskDlgChamferParameters::accept()
{
    parameter->showObject();
    parameter->apply();

    return TaskDlgDressUpParameters::accept();
}

#include "moc_TaskChamferParameters.cpp"
