/****************************************************************************
 *   Copyright (c) 2023 Zheng Lei (realthunder) <realthunder.dev@gmail.com> *
 *                                                                          *
 *   This file is part of the FreeCAD CAx development system.               *
 *                                                                          *
 *   This library is free software; you can redistribute it and/or          *
 *   modify it under the terms of the GNU Library General Public            *
 *   License as published by the Free Software Foundation; either           *
 *   version 2 of the License, or (at your option) any later version.       *
 *                                                                          *
 *   This library  is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *   GNU Library General Public License for more details.                   *
 *                                                                          *
 *   You should have received a copy of the GNU Library General Public      *
 *   License along with this library; see the file COPYING.LIB. If not,     *
 *   write to the Free Software Foundation, Inc., 59 Temple Place,          *
 *   Suite 330, Boston, MA  02111-1307, USA                                 *
 *                                                                          *
 ****************************************************************************/

#ifndef SHEET_PARAMS_H
#define SHEET_PARAMS_H

#include <Mod/Spreadsheet/SpreadsheetGlobal.h>

/*[[[cog
import SheetParams
SheetParams.declare()
]]]*/

// Auto generated code (Tools/params_utils.py:82)
#include <Base/Parameter.h>


// Auto generated code (Tools/params_utils.py:90)
namespace Spreadsheet {
/** Convenient class to obtain spreadsheet related parameters

 * The parameters are under group "User parameter:BaseApp/Preferences/Mod/Spreadsheet"
 *
 * This class is auto generated by Mod/Spreadsheet/App/SheetParams.py. Modify that file
 * instead of this one, if you want to add any parameter. You need
 * to install Cog Python package for code generation:
 * @code
 *     pip install cogapp
 * @endcode
 *
 * Once modified, you can regenerate the header and the source file,
 * @code
 *     python3 -m cogapp -r Mod/Spreadsheet/App/SheetParams.h Mod/Spreadsheet/App/SheetParams.cpp
 * @endcode
 *
 * You can add a new parameter by adding lines in Mod/Spreadsheet/App/SheetParams.py. Available
 * parameter types are 'Int, UInt, String, Bool, Float'. For example, to add
 * a new Int type parameter,
 * @code
 *     ParamInt(parameter_name, default_value, documentation, on_change=False)
 * @endcode
 *
 * If there is special handling on parameter change, pass in on_change=True.
 * And you need to provide a function implementation in Mod/Spreadsheet/App/SheetParams.cpp with
 * the following signature.
 * @code
 *     void SheetParams:on<parameter_name>Changed()
 * @endcode
 */
class SpreadsheetExport SheetParams {
public:
    static ParameterGrp::handle getHandle();

    // Auto generated code (Tools/params_utils.py:138)
    //@{
    /// Accessor for parameter showAliasName
    static const bool & getshowAliasName();
    static const bool & defaultshowAliasName();
    static void removeshowAliasName();
    static void setshowAliasName(const bool &v);
    static const char *docshowAliasName();
    //@}

    // Auto generated code (Tools/params_utils.py:138)
    //@{
    /// Accessor for parameter DisplayAliasFormatString
    static const std::string & getDisplayAliasFormatString();
    static const std::string & defaultDisplayAliasFormatString();
    static void removeDisplayAliasFormatString();
    static void setDisplayAliasFormatString(const std::string &v);
    static const char *docDisplayAliasFormatString();
    //@}

    // Auto generated code (Tools/params_utils.py:138)
    //@{
    /// Accessor for parameter AliasedCellBackgroundColor
    static const std::string & getAliasedCellBackgroundColor();
    static const std::string & defaultAliasedCellBackgroundColor();
    static void removeAliasedCellBackgroundColor();
    static void setAliasedCellBackgroundColor(const std::string &v);
    static const char *docAliasedCellBackgroundColor();
    //@}

    // Auto generated code (Tools/params_utils.py:138)
    //@{
    /// Accessor for parameter LockedAliasedCellColor
    static const std::string & getLockedAliasedCellColor();
    static const std::string & defaultLockedAliasedCellColor();
    static void removeLockedAliasedCellColor();
    static void setLockedAliasedCellColor(const std::string &v);
    static const char *docLockedAliasedCellColor();
    //@}

    // Auto generated code (Tools/params_utils.py:138)
    //@{
    /// Accessor for parameter TextColor
    static const std::string & getTextColor();
    static const std::string & defaultTextColor();
    static void removeTextColor();
    static void setTextColor(const std::string &v);
    static const char *docTextColor();
    //@}

    // Auto generated code (Tools/params_utils.py:138)
    //@{
    /// Accessor for parameter PositiveNumberColor
    static const std::string & getPositiveNumberColor();
    static const std::string & defaultPositiveNumberColor();
    static void removePositiveNumberColor();
    static void setPositiveNumberColor(const std::string &v);
    static const char *docPositiveNumberColor();
    //@}

    // Auto generated code (Tools/params_utils.py:138)
    //@{
    /// Accessor for parameter NegativeNumberColor
    static const std::string & getNegativeNumberColor();
    static const std::string & defaultNegativeNumberColor();
    static void removeNegativeNumberColor();
    static void setNegativeNumberColor(const std::string &v);
    static const char *docNegativeNumberColor();
    //@}

// Auto generated code (Tools/params_utils.py:178)
}; // class SheetParams
} // namespace Spreadsheet
//[[[end]]]
#endif // SHEET_PARAMS_H