/***************************************************************************
 *   Copyright (c) 2020 Zheng Lei (realthunder)<realthunder.dev@gmail.com> *
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

#ifndef PARTGUI_ViewProviderGenericPattern_H
#define PARTGUI_ViewProviderGenericPattern_H

#include "ViewProviderTransformed.h"

namespace PartDesignGui {

class PartDesignGuiExport ViewProviderGenericPattern : public ViewProviderTransformed
{
    Q_DECLARE_TR_FUNCTIONS(PartDesignGuii::ViewProviderGenericPattern)
    PROPERTY_HEADER(PartDesignGui::ViewProviderGenericPattern);
public:
    ViewProviderGenericPattern()
        { sPixmap = "PartDesign_GenericPattern.svg"; }

    const std::string & featureName() const override;

    QString getMenuName() const override {
        return tr("GenericPattern parameters");
    }

protected:
    /// Returns a newly create dialog for the part to be placed in the task view
    virtual TaskDlgFeatureParameters *getEditDialog();

};


} // namespace PartDesignGui


#endif // PARTGUI_ViewProviderGenericPattern_H
