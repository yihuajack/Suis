VERSION 1.0 CLASS
BEGIN
  MultiUse = -1  'True
END
Attribute VB_Name = "ThisWorkbook"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = True
Option Explicit

Private Sub Workbook_Open()
'This routine must not read or change any spreadsheet values or graphs and must
'leave bEnableChangeHandler = False, to ensure broad cross-platform start-up compatibility.
    If ActiveSheet.Name <> "Dashboard" Then Worksheets("Dashboard").Activate
    bEnableChangeHandlers = False
    bStartup = True     'Used in Sheet2 (Dashboard), set to False in ResetAll subroutine and when any spreadsheet cell is altered.

End Sub

Private Sub Workbook_Activate()
'This routine runs after the workbook has fully opened, and may include executable code that does not affect graphics.
    Call InitializeAll
    bEnableChangeHandlers = True
    bNeedReset = True
    Range(progressCell).Value = ""
    Range(errorCell).Value = ""
    Range(iterationCell).Value = ""
    
End Sub

