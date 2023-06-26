Attribute VB_Name = "PlotConstructcors"
Option Explicit
Const colorMetalN = 0.25, colorN = 0.35, colorUndoped = 0.45, colorP = 0.55, colorMetalP = 0.65
'All color schemes are selected such that lighter shades are more positive (less negative) values
'Intermediate results are plotted in gray, results after convergence failure are plotted in red
'Otherwise, electrons and photogeneration are yellow, holes are blue, all others green

Sub RefreshCharts()
If bMac Then  'DoEvents is needed for immediate display of plot results on Windows, but it crashes Mac!
    Application.Calculation = xlCalculationAutomatic
Else
    DoEvents: DoEvents: DoEvents
End If
End Sub

Sub OnPlotChange()
Call MapInputParameters
If bNeedReset And Not bScan And Not bBatch And currentPlot <> "Layout" And currentPlot <> "Illumination" Then
    MsgBox ("3D plot details only available after you have Solved the problem.")
    Exit Sub
End If
If currentPlot = "Layout" Then Call PlotLayout
If currentPlot = "Illumination" Then Call PlotIllumination
If currentPlot = "Surface Recomb" Then Call PlotSurfaceRecomb
If currentPlot = "Surface Current" Then Call PlotSurfaceCurrent
If currentPlot = "Luminescence" Then Call PlotLuminescence
If currentPlot = "Conductance" Then Call PlotConductance
If currentPlot = "Electrothermal" Then Call PlotThermal
If currentPlot = "Generation" Then Call PlotGeneration
If currentPlot = "Bulk Recomb" Then Call PlotRecombination
If currentPlot = "Excess Carriers" Then Call PlotGamma
If currentPlot = "Excess Psi" Then Call PlotPsi
If currentPlot = "Quasi-Fermi" Then Call PlotPhi
If currentPlot = "Bulk Current" Then Call PlotCurrent
End Sub

Sub UpdateContourChart()
'Changing the legend colors forces Excel to refresh the chart
Range(unitCell).Value = Range(plotUnitsCell)
Range(minCell).Value = Range(plotPlaneMinCell)
Range(maxCell).Value = Range(plotPlaneMaxCell)
If currentPlot = "Layout" Then  'Colorful scheme
    Worksheets("Dashboard").ChartObjects(contourPlot).Chart.SetElement (msoElementLegendBottom)
    Worksheets("Dashboard").ChartObjects(contourPlot).Chart.Legend.LegendEntries(1).LegendKey.Interior.Color = RGB(0, 176, 240)
    Worksheets("Dashboard").ChartObjects(contourPlot).Chart.Legend.LegendEntries(2).LegendKey.Interior.Color = RGB(30, 30, 30)
    Worksheets("Dashboard").ChartObjects(contourPlot).Chart.Legend.LegendEntries(3).LegendKey.Interior.Color = RGB(163, 163, 163)
    Worksheets("Dashboard").ChartObjects(contourPlot).Chart.Legend.LegendEntries(4).LegendKey.Interior.Color = RGB(255, 191, 0)
    Worksheets("Dashboard").ChartObjects(contourPlot).Chart.Legend.LegendEntries(5).LegendKey.Interior.Color = RGB(112, 171, 70)
    Worksheets("Dashboard").ChartObjects(contourPlot).Chart.Legend.LegendEntries(6).LegendKey.Interior.Color = RGB(90, 153, 211)
    Worksheets("Dashboard").ChartObjects(contourPlot).Chart.Legend.LegendEntries(7).LegendKey.Interior.Color = RGB(140, 140, 140)
    Worksheets("Dashboard").ChartObjects(contourPlot).Chart.Legend.LegendEntries(8).LegendKey.Interior.Color = RGB(200, 200, 200)
    Worksheets("Dashboard").ChartObjects(contourPlot).Chart.Legend.LegendEntries(9).LegendKey.Interior.Color = RGB(220, 220, 220)
    Worksheets("Dashboard").ChartObjects(contourPlot).Chart.Legend.LegendEntries(10).LegendKey.Interior.Color = RGB(255, 255, 255)
    Worksheets("Dashboard").ChartObjects(contourPlot).Chart.Legend.Delete
    Call RefreshCharts
    Exit Sub
End If
'Default green scheme
    Worksheets("Dashboard").ChartObjects(contourPlot).Chart.SetElement (msoElementLegendBottom)
    Worksheets("Dashboard").ChartObjects(contourPlot).Chart.Legend.LegendEntries(1).LegendKey.Interior.Color = RGB(72, 114, 44)
    Worksheets("Dashboard").ChartObjects(contourPlot).Chart.Legend.LegendEntries(2).LegendKey.Interior.Color = RGB(82, 129, 50)
    Worksheets("Dashboard").ChartObjects(contourPlot).Chart.Legend.LegendEntries(3).LegendKey.Interior.Color = RGB(90, 141, 56)
    Worksheets("Dashboard").ChartObjects(contourPlot).Chart.Legend.LegendEntries(4).LegendKey.Interior.Color = RGB(98, 151, 62)
    Worksheets("Dashboard").ChartObjects(contourPlot).Chart.Legend.LegendEntries(5).LegendKey.Interior.Color = RGB(104, 161, 66)
    Worksheets("Dashboard").ChartObjects(contourPlot).Chart.Legend.LegendEntries(6).LegendKey.Interior.Color = RGB(112, 171, 70)
    Worksheets("Dashboard").ChartObjects(contourPlot).Chart.Legend.LegendEntries(7).LegendKey.Interior.Color = RGB(137, 183, 112)
    Worksheets("Dashboard").ChartObjects(contourPlot).Chart.Legend.LegendEntries(8).LegendKey.Interior.Color = RGB(159, 195, 143)
    Worksheets("Dashboard").ChartObjects(contourPlot).Chart.Legend.LegendEntries(9).LegendKey.Interior.Color = RGB(179, 205, 167)
    Worksheets("Dashboard").ChartObjects(contourPlot).Chart.Legend.LegendEntries(10).LegendKey.Interior.Color = RGB(193, 215, 185)
    Worksheets("Dashboard").ChartObjects(contourPlot).Chart.Legend.Delete
If currentPlot = "Generation" Or currentPlot = "Illumination" Or _
   currentPlot = "Luminescence" Or currentPlot = "Electrothermal" Then   'yellow scheme
    Worksheets("Dashboard").ChartObjects(contourPlot).Chart.SetElement (msoElementLegendBottom)
    Worksheets("Dashboard").ChartObjects(contourPlot).Chart.Legend.LegendEntries(1).LegendKey.Interior.Color = RGB(80, 80, 30)
    Worksheets("Dashboard").ChartObjects(contourPlot).Chart.Legend.LegendEntries(2).LegendKey.Interior.Color = RGB(100, 100, 0)
    Worksheets("Dashboard").ChartObjects(contourPlot).Chart.Legend.LegendEntries(3).LegendKey.Interior.Color = RGB(125, 125, 0)
    Worksheets("Dashboard").ChartObjects(contourPlot).Chart.Legend.LegendEntries(4).LegendKey.Interior.Color = RGB(150, 150, 0)
    Worksheets("Dashboard").ChartObjects(contourPlot).Chart.Legend.LegendEntries(5).LegendKey.Interior.Color = RGB(175, 175, 0)
    Worksheets("Dashboard").ChartObjects(contourPlot).Chart.Legend.LegendEntries(6).LegendKey.Interior.Color = RGB(200, 200, 0)
    Worksheets("Dashboard").ChartObjects(contourPlot).Chart.Legend.LegendEntries(7).LegendKey.Interior.Color = RGB(225, 225, 0)
    Worksheets("Dashboard").ChartObjects(contourPlot).Chart.Legend.LegendEntries(8).LegendKey.Interior.Color = RGB(250, 250, 0)
    Worksheets("Dashboard").ChartObjects(contourPlot).Chart.Legend.LegendEntries(9).LegendKey.Interior.Color = RGB(255, 255, 100)
    Worksheets("Dashboard").ChartObjects(contourPlot).Chart.Legend.LegendEntries(10).LegendKey.Interior.Color = RGB(255, 255, 150)
    Worksheets("Dashboard").ChartObjects(contourPlot).Chart.Legend.Delete
    If currentPlot = "Generation" Or currentPlot = "Illumination" Then
        Call RefreshCharts
        Exit Sub
    End If
End If
If (currentPlot = "Quasi-Fermi" And selectQF = "Hole") Or _
   (currentPlot = "Bulk Current" And selectJCarrier = "Hole") Then    'blue scheme
    Worksheets("Dashboard").ChartObjects(contourPlot).Chart.SetElement (msoElementLegendBottom)
    Worksheets("Dashboard").ChartObjects(contourPlot).Chart.Legend.LegendEntries(1).LegendKey.Interior.Color = RGB(58, 102, 141)
    Worksheets("Dashboard").ChartObjects(contourPlot).Chart.Legend.LegendEntries(2).LegendKey.Interior.Color = RGB(66, 116, 159)
    Worksheets("Dashboard").ChartObjects(contourPlot).Chart.Legend.LegendEntries(3).LegendKey.Interior.Color = RGB(74, 126, 175)
    Worksheets("Dashboard").ChartObjects(contourPlot).Chart.Legend.LegendEntries(4).LegendKey.Interior.Color = RGB(80, 135, 187)
    Worksheets("Dashboard").ChartObjects(contourPlot).Chart.Legend.LegendEntries(5).LegendKey.Interior.Color = RGB(86, 145, 199)
    Worksheets("Dashboard").ChartObjects(contourPlot).Chart.Legend.LegendEntries(6).LegendKey.Interior.Color = RGB(90, 153, 211)
    Worksheets("Dashboard").ChartObjects(contourPlot).Chart.Legend.LegendEntries(7).LegendKey.Interior.Color = RGB(124, 169, 217)
    Worksheets("Dashboard").ChartObjects(contourPlot).Chart.Legend.LegendEntries(8).LegendKey.Interior.Color = RGB(149, 183, 223)
    Worksheets("Dashboard").ChartObjects(contourPlot).Chart.Legend.LegendEntries(9).LegendKey.Interior.Color = RGB(171, 197, 227)
    Worksheets("Dashboard").ChartObjects(contourPlot).Chart.Legend.LegendEntries(10).LegendKey.Interior.Color = RGB(189, 207, 233)
    Worksheets("Dashboard").ChartObjects(contourPlot).Chart.Legend.Delete
End If
If (currentPlot = "Quasi-Fermi" And selectQF = "Electron") Or _
   (currentPlot = "Bulk Current" And selectJCarrier = "Electron") Then    'orange scheme
    Worksheets("Dashboard").ChartObjects(contourPlot).Chart.SetElement (msoElementLegendBottom)
    Worksheets("Dashboard").ChartObjects(contourPlot).Chart.Legend.LegendEntries(1).LegendKey.Interior.Color = RGB(171, 128, 0)
    Worksheets("Dashboard").ChartObjects(contourPlot).Chart.Legend.LegendEntries(2).LegendKey.Interior.Color = RGB(191, 143, 0)
    Worksheets("Dashboard").ChartObjects(contourPlot).Chart.Legend.LegendEntries(3).LegendKey.Interior.Color = RGB(209, 157, 0)
    Worksheets("Dashboard").ChartObjects(contourPlot).Chart.Legend.LegendEntries(4).LegendKey.Interior.Color = RGB(225, 169, 0)
    Worksheets("Dashboard").ChartObjects(contourPlot).Chart.Legend.LegendEntries(5).LegendKey.Interior.Color = RGB(239, 179, 0)
    Worksheets("Dashboard").ChartObjects(contourPlot).Chart.Legend.LegendEntries(6).LegendKey.Interior.Color = RGB(255, 191, 0)
    Worksheets("Dashboard").ChartObjects(contourPlot).Chart.Legend.LegendEntries(7).LegendKey.Interior.Color = RGB(255, 199, 92)
    Worksheets("Dashboard").ChartObjects(contourPlot).Chart.Legend.LegendEntries(8).LegendKey.Interior.Color = RGB(255, 207, 131)
    Worksheets("Dashboard").ChartObjects(contourPlot).Chart.Legend.LegendEntries(9).LegendKey.Interior.Color = RGB(255, 215, 159)
    Worksheets("Dashboard").ChartObjects(contourPlot).Chart.Legend.LegendEntries(10).LegendKey.Interior.Color = RGB(255, 223, 179)
    Worksheets("Dashboard").ChartObjects(contourPlot).Chart.Legend.Delete
End If
If bConvergenceFailure Then 'red scheme
    Worksheets("Dashboard").ChartObjects(contourPlot).Chart.SetElement (msoElementLegendBottom)
    Worksheets("Dashboard").ChartObjects(contourPlot).Chart.Legend.LegendEntries(1).LegendKey.Interior.Color = RGB(157, 82, 28)
    Worksheets("Dashboard").ChartObjects(contourPlot).Chart.Legend.LegendEntries(2).LegendKey.Interior.Color = RGB(177, 92, 34)
    Worksheets("Dashboard").ChartObjects(contourPlot).Chart.Legend.LegendEntries(3).LegendKey.Interior.Color = RGB(195, 102, 38)
    Worksheets("Dashboard").ChartObjects(contourPlot).Chart.Legend.LegendEntries(4).LegendKey.Interior.Color = RGB(209, 110, 42)
    Worksheets("Dashboard").ChartObjects(contourPlot).Chart.Legend.LegendEntries(5).LegendKey.Interior.Color = RGB(223, 118, 46)
    Worksheets("Dashboard").ChartObjects(contourPlot).Chart.Legend.LegendEntries(6).LegendKey.Interior.Color = RGB(235, 124, 48)
    Worksheets("Dashboard").ChartObjects(contourPlot).Chart.Legend.LegendEntries(7).LegendKey.Interior.Color = RGB(237, 145, 104)
    Worksheets("Dashboard").ChartObjects(contourPlot).Chart.Legend.LegendEntries(8).LegendKey.Interior.Color = RGB(239, 165, 137)
    Worksheets("Dashboard").ChartObjects(contourPlot).Chart.Legend.LegendEntries(9).LegendKey.Interior.Color = RGB(243, 183, 163)
    Worksheets("Dashboard").ChartObjects(contourPlot).Chart.Legend.LegendEntries(10).LegendKey.Interior.Color = RGB(243, 197, 183)
    Worksheets("Dashboard").ChartObjects(contourPlot).Chart.Legend.Delete
End If
If bIntermediatePlot Then   'gray scheme
    Worksheets("Dashboard").ChartObjects(contourPlot).Chart.SetElement (msoElementLegendBottom)
    Worksheets("Dashboard").ChartObjects(contourPlot).Chart.Legend.LegendEntries(1).LegendKey.Interior.Color = RGB(108, 108, 108)
    Worksheets("Dashboard").ChartObjects(contourPlot).Chart.Legend.LegendEntries(2).LegendKey.Interior.Color = RGB(122, 122, 122)
    Worksheets("Dashboard").ChartObjects(contourPlot).Chart.Legend.LegendEntries(3).LegendKey.Interior.Color = RGB(135, 135, 135)
    Worksheets("Dashboard").ChartObjects(contourPlot).Chart.Legend.LegendEntries(4).LegendKey.Interior.Color = RGB(145, 145, 145)
    Worksheets("Dashboard").ChartObjects(contourPlot).Chart.Legend.LegendEntries(5).LegendKey.Interior.Color = RGB(155, 155, 155)
    Worksheets("Dashboard").ChartObjects(contourPlot).Chart.Legend.LegendEntries(6).LegendKey.Interior.Color = RGB(163, 163, 163)
    Worksheets("Dashboard").ChartObjects(contourPlot).Chart.Legend.LegendEntries(7).LegendKey.Interior.Color = RGB(177, 177, 177)
    Worksheets("Dashboard").ChartObjects(contourPlot).Chart.Legend.LegendEntries(8).LegendKey.Interior.Color = RGB(181, 181, 181)
    Worksheets("Dashboard").ChartObjects(contourPlot).Chart.Legend.LegendEntries(9).LegendKey.Interior.Color = RGB(201, 201, 201)
    Worksheets("Dashboard").ChartObjects(contourPlot).Chart.Legend.LegendEntries(10).LegendKey.Interior.Color = RGB(211, 211, 211)
    Worksheets("Dashboard").ChartObjects(contourPlot).Chart.Legend.Delete
End If
Call RefreshCharts
End Sub

Sub ValidatePosition(ByVal thisPlot As String)
Dim k As Integer
Dim rPosition As Double
bEnableChangeHandlers = False
If thisPlot = "Layout" Or thisPlot = "Illumination" Or _
    thisPlot = "Surface Recomb" Or thisPlot = "Surface Current" Or _
    thisPlot = "Luminescence" Or thisPlot = "Conductance" Or thisPlot = "Electrothermal" Then       'Surface plot
    If Range(viewCell) <> "Surface" Then Range(viewCell).Value = "Surface"
    If Range(positionCell) <> "" Then Range(positionCell).Value = ""
Else                                                                 'Bulk plot
    If Range(viewCell) = "Surface" Then Range(viewCell).Value = Range(lastPlotViewCell)
    If Range(positionCell) < 0 Or Range(positionCell) = "" Or Not IsNumeric(Range(positionCell)) Then Range(positionCell).Value = 0
    If Range(viewCell) = "Top" And Range(positionCell) > 10 * deviceHeight Then Range(positionCell).Value = 10 * deviceHeight
    If Range(viewCell) = "Face" And Range(positionCell) > 10 * deviceLength Then Range(positionCell).Value = 10 * deviceLength
    If Range(viewCell) = "Side" And Range(positionCell) > 10 * deviceWidth Then Range(positionCell).Value = 10 * deviceWidth
End If
bEnableChangeHandlers = True
End Sub

Sub ScanPlot()
Dim i As Integer, j As Integer, k As Integer, n As Integer, nScanSteps As Integer
Dim rEndTime As Single
Dim rPosition As Double, maxPosition As Double    'mm
Dim bSaveVolumeMin As Boolean, bSaveVolumeMax As Boolean
Dim saveSurface As String
Dim starttime As Single
Call MapInputParameters
If bNeedReset And currentPlot <> "Layout" And currentPlot <> "Illumination" Then
    MsgBox ("Scan available only after you have Solved the problem.")
    Exit Sub
End If
If currentPlot = "Conductance" Or currentPlot = "Electrothermal" Then Exit Sub
If currentPlot = "Layout" Or currentPlot = "Illumination" Or _
   currentPlot = "Surface Recomb" Or currentPlot = "Surface Current" Or _
   currentPlot = "Illumination" Or currentPlot = "Luminescence" Then
    saveSurface = selectSurface
    If saveSurface <> "Front" Then
        starttime = Timer
        Range(selectSurfaceCell).Value = "Front": DoEvents
        RefreshCharts
        Do While Timer - starttime < 1: Loop    'One second showing front surface
    End If
    If saveSurface <> "Rear" Then
        starttime = Timer
        Range(selectSurfaceCell).Value = "Rear": DoEvents
        RefreshCharts
        Do While Timer - starttime < 1: Loop    'One second showing rear surface
    End If
    Range(selectSurfaceCell).Value = saveSurface: DoEvents  'Restore original view
    RefreshCharts
    Exit Sub
End If
rPosition = Range(positionCell)       'mm
maxPosition = 0
'Note that displayed dimensions are mostly in mm, but stored dimensions are in cm for equation solver
If Range(viewCell) = "Top" Then maxPosition = 10 * deviceHeight
If Range(viewCell) = "Face" Then maxPosition = 10 * deviceLength
If Range(viewCell) = "Side" Then maxPosition = 10 * deviceWidth
If b3D Or Range(viewCell) = "Top" Or Range(viewCell) = "Side" Then nScanSteps = nFlyThroughSteps Else nScanSteps = 1
bVolumeScale = True
rEndTime = Timer + 1    'Wait at zero position with revised volume scaling before starting scan
Range(positionCell).Value = 0: DoEvents 'Triggers OnPlotChange
RefreshCharts
Do While Timer < rEndTime: Loop
For k = 1 To nScanSteps
    rEndTime = Timer + 0.1  'Don't update faster than 10 frames per second
    Range(positionCell).Value = Round(k * maxPosition / nScanSteps, 3): DoEvents 'Triggers OnPlotChange
    If bMac Then RefreshCharts      'Nothing additional needed here for Windows
    Do While Timer < rEndTime: Loop
Next k
rEndTime = Timer + 1    'Wait before restoring pre-scan position
Do While Timer < rEndTime: Loop
bVolumeScale = False
Range(positionCell).Value = rPosition: DoEvents   'Triggers OnPlotChange
RefreshCharts
End Sub

Sub PostData(ByVal rVolumeMin As Double, ByVal rVolumeMax As Double)
'Posts the current contents of sPlot in columnar and tabular format on the Data! sheet
'Also clears max-extent of normalized data range
Dim i As Integer, j As Integer, row As Integer
Dim nAbscissaEntries As Integer, nOrdinateEntries As Integer
Dim maxAbscissa As Double, maxOrdinate As Double
Dim columnHeader As Variant, columnData As Variant, abscissaData As Variant, ordinateData As Variant
Range(columnarDataCells).ClearContents
Range(tabularDataCells).ClearContents
Range(tabularAbscissaCells).ClearContents
Range(tabularOrdinateCells).ClearContents
columnHeader = Range(columnarHeaderCells)
columnData = Range(columnarDataCells)
abscissaData = Range(tabularAbscissaCells)
ordinateData = Range(tabularOrdinateCells)
Range(plotSurfaceCell).Value = ""
Range(plotCarrierCell).Value = ""
Range(plotDirectionCell).Value = ""
Range(plotPortionCell).Value = ""
Range(tabularUnitsCell).Value = "mm"
If Range(viewCell) = "Top" Or Range(viewCell) = "Surface" Then
    maxAbscissa = 10 * deviceWidth
    maxOrdinate = 10 * deviceLength
    columnHeader(1, 1) = "X(mm)"
    columnHeader(1, 2) = "Y(mm)"
    nAbscissaEntries = nNodesX
    nOrdinateEntries = nNodesY
End If
If Range(viewCell) = "Face" Then
    maxAbscissa = 10 * deviceWidth
    maxOrdinate = 10 * deviceHeight
    columnHeader(1, 1) = "X(mm)"
    columnHeader(1, 2) = "Z(mm)"
    nAbscissaEntries = nNodesX
    nOrdinateEntries = nNodesZ
End If
If Range(viewCell) = "Side" Then
    maxAbscissa = 10 * deviceLength
    maxOrdinate = 10 * deviceHeight
    columnHeader(1, 1) = "Y(mm)"
    columnHeader(1, 2) = "Z(mm)"
    nAbscissaEntries = nNodesY
    nOrdinateEntries = nNodesZ
End If
Range(columnarHeaderCells) = columnHeader
'Fill abscissa and ordinate fields for tabular data
For i = 1 To nAbscissaEntries
    abscissaData(1, i) = maxAbscissa * (i - 1) / (nAbscissaEntries - 1)
Next i
Range(tabularAbscissaCells) = abscissaData
For j = 1 To nOrdinateEntries
    ordinateData(j, 1) = maxOrdinate * (j - 1) / (nOrdinateEntries - 1)
Next j
Range(tabularOrdinateCells) = ordinateData
'Fill columnar data fields
row = 0
For i = 1 To nAbscissaEntries: For j = 1 To nOrdinateEntries
    row = row + 1
    columnData(row, 1) = maxAbscissa * (i - 1) / (nAbscissaEntries - 1)
    columnData(row, 2) = maxOrdinate * (j - 1) / (nOrdinateEntries - 1)
    columnData(row, 3) = sPlot(j, i)
Next j: Next i
Range(columnarDataCells) = columnData
For i = 1 To nMaxNodes: For j = 1 To nMaxNodes
    If i <= UBound(sPlot, 2) And j <= UBound(sPlot, 1) Then Range(tabularDataCells).Cells(j, i) = sPlot(j, i)
Next j: Next i
'Fill columnar metadata fields
Range(plotNameCell).Value = currentPlot
Range(plotViewCell).Value = Range(viewCell)
Range(plotPositionCell).Value = Range(positionCell)
Range(plotPlaneMinCell).Value = WorksheetFunction.Min(sPlot)
Range(plotPlaneMaxCell).Value = WorksheetFunction.max(sPlot)
Range(plotVolumeMinCell).Value = rVolumeMin
Range(plotVolumeMaxCell).Value = rVolumeMax
Range(plotResolutionCell).Value = Range(resolutionCell)
If currentPlot = "Layout" Or currentPlot = "Illumination" Or currentPlot = "Surface Recomb" Or _
   currentPlot = "Surface Current" Or currentPlot = "Luminescence" Then Range(plotSurfaceCell).Value = selectSurface
If currentPlot = "Conductance" Or currentPlot = "Electrothermal" Then Range(plotSurfaceCell).Value = "Front"
If currentPlot = "Quasi-Fermi" Then Range(plotCarrierCell).Value = selectQF
If currentPlot = "Bulk Current" Then
    Range(plotCarrierCell).Value = selectJCarrier
    Range(plotDirectionCell).Value = selectJDirection
    Range(plotPortionCell).Value = selectJPortion
End If
End Sub

Sub PlotLayout()
Dim i As Integer, j As Integer, nSurface As Integer
Dim m As Integer, n As Integer
ReDim sPlot(1 To nMaxNodes, 1 To nMaxNodes) As Double
If bNeedReset Then Call ResetAll
Call MapInputParameters
If selectSurface = "Front" Then nSurface = front Else nSurface = rear
For m = 1 To nMaxNodes: For n = 1 To nMaxNodes
    i = Round((m - 1) * (nNodesX - 1) / (nMaxNodes - 1), 0) + 1
    j = Round((n - 1) * (nNodesY - 1) / (nMaxNodes - 1), 0) + 1
    If sSigma(nSurface, i, j) < 0 Then
        sPlot(n, m) = colorN
    Else
        If sSigma(nSurface, i, j) > 0 Then
            sPlot(n, m) = colorP
        Else
            sPlot(n, m) = colorUndoped
        End If
    End If
    If sContact(nSurface, holes, i, j) > 0 Or sSigma(nSurface, i, j) > 1 Then sPlot(n, m) = colorMetalP
    If sContact(nSurface, electrons, i, j) > 0 Or sSigma(nSurface, i, j) < -1 Then sPlot(n, m) = colorMetalN
Next n: Next m
currentPlot = "Layout"
Range(titleCell).Value = currentPlot
Call ValidatePosition(currentPlot)
Range(columnarDataCells).ClearContents
Range(tabularDataCells).ClearContents
Range(normalizedDataCells).ClearContents
Range(normalizedDataCells) = sPlot
Range(plotNameCell).Value = currentPlot
Range(plotViewCell).Value = "Top"
Range(plotPositionCell).Value = ""
Range(plotUnitsCell).Value = "": Range(unitCell).Value = ""
Range(plotPlaneMinCell).Value = "": Range(minCell).Value = ""
Range(plotPlaneMaxCell).Value = "": Range(maxCell).Value = ""
Range(plotVolumeMinCell).Value = ""
Range(plotVolumeMaxCell).Value = ""
Range(plotResolutionCell).Value = Range(resolutionCell)
Range(plotSurfaceCell).Value = selectSurface
Range(plotCarrierCell).Value = ""
Range(plotDirectionCell).Value = ""
Range(plotPortionCell).Value = ""
Call UpdateContourChart
End Sub

Sub PlotIllumination()
Dim i As Integer, j As Integer
Dim rPlaneMin As Double, rPlaneMax As Double
ReDim sPlot(1 To nNodesY, 1 To nNodesX) As Double
If bNeedReset Then Call ResetAll
Call MapInputParameters
If selectSurface = "Front" Then Call CalculateIlluminationUniformity(front, sFrontIllumination)
If selectSurface = "Rear" Then Call CalculateIlluminationUniformity(rear, sRearIllumination)
currentPlot = "Illumination"
Range(titleCell).Value = currentPlot
Call ValidatePosition(currentPlot)
For i = 1 To nNodesX: For j = 1 To nNodesY
    sPlot(j, i) = 0
    If bSpectrum Then
        If selectSurface = "Front" Then sPlot(j, i) = sPlot(j, i) + sFrontIllumination(i, j) * rFrontSuns
        If selectSurface = "Rear" Then sPlot(j, i) = sPlot(j, i) + sRearIllumination(i, j) * rRearSuns
    End If
    If bMono Then
        If selectSurface = "Front" Then sPlot(j, i) = sPlot(j, i) + sFrontIllumination(i, j) * rFrontMonoPower
        If selectSurface = "Rear" Then sPlot(j, i) = sPlot(j, i) + sRearIllumination(i, j) * rRearMonoPower
    End If
Next j: Next i
If bSpectrum Then Range(plotUnitsCell).Value = "suns"
If bMono Then Range(plotUnitsCell).Value = "W/cm2"
If bSpectrum And bMono Then Range(plotUnitsCell).Value = "mixed"
rPlaneMin = WorksheetFunction.Min(sPlot)
rPlaneMax = WorksheetFunction.max(sPlot)
Call PostData(rPlaneMin, rPlaneMax)
Call NormalizePlot(sPlot, rPlaneMin, rPlaneMax)
Range(normalizedDataCells) = sPlot
Range(plotSurfaceCell).Value = selectSurface
Call UpdateContourChart
End Sub

Sub PlotSurfaceRecomb()
Dim i As Integer, j As Integer
Dim rPlaneMin As Double, rPlaneMax As Double
ReDim sPlot(1 To nNodesY, 1 To nNodesX) As Double
If Not bGamma Then
    If Not bScan And Not bBatch Then MsgBox ("This plot available only after you have Solved the problem.")
    Exit Sub
End If
currentPlot = "Surface Recomb"
Range(titleCell).Value = currentPlot
Call ValidatePosition(currentPlot)
For i = 1 To nNodesX: For j = 1 To nNodesY
    If selectSurface = "Front" Then sPlot(j, i) = q * sFrontRecomb(i, j)
    If selectSurface = "Rear" Then sPlot(j, i) = q * sRearRecomb(i, j)
Next j: Next i
rPlaneMin = WorksheetFunction.Min(sPlot)
rPlaneMax = WorksheetFunction.max(sPlot)
Call PostData(rPlaneMin, rPlaneMax)
Call NormalizePlot(sPlot, rPlaneMin, rPlaneMax)
Range(normalizedDataCells) = sPlot
Range(plotSurfaceCell).Value = selectSurface
Range(plotUnitsCell).Value = "A/cm2"
Call UpdateContourChart
End Sub

Sub PlotSurfaceCurrent()
Dim i As Integer, j As Integer, nSurface As Integer
Dim rPlaneMin As Double, rPlaneMax As Double
ReDim sPlot(1 To nNodesY, 1 To nNodesX) As Double
If Not bGamma Then
    If Not bScan And Not bBatch Then MsgBox ("This plot available only after you have Solved the problem.")
    Exit Sub
End If
If selectSurface = "Front" Then nSurface = front Else nSurface = rear
currentPlot = "Surface Current"
Range(titleCell).Value = currentPlot
Call ValidatePosition(currentPlot)
For i = 1 To nNodesX: For j = 1 To nNodesY
    sPlot(j, i) = Sqr(LateralNodeCurrentX(nSurface, i, j) ^ 2 + LateralNodeCurrentY(nSurface, i, j) ^ 2)
Next j: Next i
rPlaneMin = WorksheetFunction.Min(sPlot)
rPlaneMax = WorksheetFunction.max(sPlot)
Call PostData(rPlaneMin, rPlaneMax)
Call NormalizePlot(sPlot, rPlaneMin, rPlaneMax)
Range(normalizedDataCells) = sPlot
Range(plotSurfaceCell).Value = selectSurface
Range(plotUnitsCell).Value = "A/cm"
Call UpdateContourChart
End Sub

Sub PlotLuminescence()
Dim i As Integer, j As Integer, k As Integer
Dim rPlaneMin As Double, rPlaneMax As Double
ReDim sPlot(1 To nNodesY, 1 To nNodesX) As Double
If Not bGamma Then
    If Not bScan And Not bBatch Then MsgBox ("This plot available only after you have Solved the problem.")
    Exit Sub
End If
currentPlot = "Luminescence"
Range(titleCell).Value = currentPlot
Call ValidatePosition(currentPlot)
For i = 1 To nNodesX: For j = 1 To nNodesY
    sPlot(j, i) = 0
    For k = 1 To nNodesZ
        sPlot(j, i) = sPlot(j, i) + (rEqP + vGamma(i, j, k)) * (rEqN + vGamma(i, j, k)) - rEqP * rEqN
    Next k
    If selectSurface = "Front" Then sPlot(j, i) = sPlot(j, i) * sFrontTransmission(i, j)
    If selectSurface = "Rear" Then sPlot(j, i) = sPlot(j, i) * sRearTransmission(i, j)
Next j: Next i
rPlaneMin = WorksheetFunction.Min(sPlot)
rPlaneMax = WorksheetFunction.max(sPlot)
If rPlaneMax > 0 Then
    For i = 1 To nNodesX: For j = 1 To nNodesY
        sPlot(j, i) = sPlot(j, i) / rPlaneMax
    Next j: Next i
    rPlaneMin = rPlaneMin / rPlaneMax
    rPlaneMax = 1
End If
Call PostData(rPlaneMin, rPlaneMax)
Call NormalizePlot(sPlot, rPlaneMin, rPlaneMax)
Range(normalizedDataCells) = sPlot
Range(plotSurfaceCell).Value = selectSurface
Range(plotUnitsCell).Value = "none"
Call UpdateContourChart
End Sub

Sub PlotConductance()
Dim i As Integer, j As Integer, k As Integer
Dim localConductivity As Double
Dim rPlaneMin As Double, rPlaneMax As Double
ReDim sPlot(1 To nNodesY, 1 To nNodesX) As Double
If Not bGamma Then
    If Not bScan And Not bBatch Then MsgBox ("This plot available only after you have Solved the problem.")
    Exit Sub
End If
currentPlot = "Conductance"
Range(titleCell).Value = currentPlot
Call ValidatePosition(currentPlot)
For i = 1 To nNodesX: For j = 1 To nNodesY
    sPlot(j, i) = 0
    For k = 1 To nNodesZ
        localConductivity = q * vGamma(i, j, k) * rDp / rVt + q * vGamma(i, j, k) * rDn / rVt
        If k = 1 Or k = nNodesZ Then localConductivity = 0.5 * localConductivity
        sPlot(j, i) = sPlot(j, i) + localConductivity * deviceHeight / (nNodesZ - 1)
    Next k
Next j: Next i
rPlaneMin = WorksheetFunction.Min(sPlot)
rPlaneMax = WorksheetFunction.max(sPlot)
Call PostData(rPlaneMin, rPlaneMax)
Call NormalizePlot(sPlot, rPlaneMin, rPlaneMax)
Range(normalizedDataCells) = sPlot
Range(plotSurfaceCell).Value = "Front"
Range(plotUnitsCell).Value = "siemens"
Call UpdateContourChart
End Sub

Sub PlotThermal()
'Plot thermal power loss (W/cm2) due to electrical (not optical) dissipation.
'Includes J-dot-GradPhi in the bulk, bulk recombination, surface recombination, lateral surface flow, and contact resistance.
Dim i As Integer, j As Integer, k As Integer
Dim localP As Double, localN As Double, localJdotGrad As Double, frontKdotGrad As Double, rearKdotGrad As Double
Dim localRV As Double, contactLoss As Double
Dim rPlaneMin As Double, rPlaneMax As Double
ReDim sPlot(1 To nNodesY, 1 To nNodesX) As Double
If Not bGamma Then
    If Not bScan And Not bBatch Then MsgBox ("This plot available only after you have Solved the problem.")
    Exit Sub
End If
If bGamma And Not bGradGammaPsi Then Call CalculateGradGammaPsi
currentPlot = "Electrothermal"
Range(titleCell).Value = currentPlot
Call ValidatePosition(currentPlot)
'Bulk losses (J dot gradPhi and recombination)
For i = 1 To nNodesX: For j = 1 To nNodesY
    sPlot(j, i) = 0
    For k = 1 To nNodesZ
        localP = rEqP + vGamma(i, j, k)
        localN = rEqN + vGamma(i, j, k)
        localJdotGrad = (-q * rDp * vGradGammaX(i, j, k) - q * rDp * localP * vGradPsiX(i, j, k) / rVt) * (vGradPsiX(i, j, k) + rVt * vGradGammaX(i, j, k) / localP) _
                    + (q * rDn * vGradGammaX(i, j, k) - q * rDn * localN * vGradPsiX(i, j, k) / rVt) * (vGradPsiX(i, j, k) - rVt * vGradGammaX(i, j, k) / localN) _
                    + (-q * rDp * vGradGammaY(i, j, k) - q * rDp * localP * vGradPsiY(i, j, k) / rVt) * (vGradPsiY(i, j, k) + rVt * vGradGammaY(i, j, k) / localP) _
                    + (q * rDn * vGradGammaY(i, j, k) - q * rDn * localN * vGradPsiY(i, j, k) / rVt) * (vGradPsiY(i, j, k) - rVt * vGradGammaY(i, j, k) / localN) _
                    + (-q * rDp * vGradGammaZ(i, j, k) - q * rDp * localP * vGradPsiZ(i, j, k) / rVt) * (vGradPsiZ(i, j, k) + rVt * vGradGammaZ(i, j, k) / localP) _
                    + (q * rDn * vGradGammaZ(i, j, k) - q * rDn * localN * vGradPsiZ(i, j, k) / rVt) * (vGradPsiZ(i, j, k) - rVt * vGradGammaZ(i, j, k) / localN)
        If k = 1 Or k = nNodesZ Then localJdotGrad = 0.5 * localJdotGrad
        sPlot(j, i) = sPlot(j, i) - deviceHeight / (nNodesZ - 1) * localJdotGrad
        localRV = q * vRecombination(i, j, k) * rVt * Log(localP * localN / rNi ^ 2)
        If k = 1 Or k = nNodesZ Then localRV = 0.5 * localRV
        sPlot(j, i) = sPlot(j, i) + deviceHeight / (nNodesZ - 1) * localRV
    Next k
'Surface recombination losses
    sPlot(j, i) = sPlot(j, i) + q * sFrontRecomb(i, j) * (sPhi(front, holes, i, j) - sPhi(front, electrons, i, j))
    sPlot(j, i) = sPlot(j, i) + q * sRearRecomb(i, j) * (sPhi(rear, holes, i, j) - sPhi(rear, electrons, i, j))
'Front K dot GradPhi
    localP = rEqP + vGamma(i, j, 1)
    localN = rEqN + vGamma(i, j, 1)
    If sSigma(front, i, j) > 0 Then    'K dot GradPhiP
        frontKdotGrad = LateralNodeCurrentX(front, i, j) * (vGradPsiX(i, j, 1) + rVt * vGradGammaX(i, j, 1) / localP) _
                                 + LateralNodeCurrentY(front, i, j) * (vGradPsiY(i, j, 1) + rVt * vGradGammaY(i, j, 1) / localP)
    Else    'K dot GradPhiN
        frontKdotGrad = LateralNodeCurrentX(front, i, j) * (vGradPsiX(i, j, 1) - rVt * vGradGammaX(i, j, 1) / localN) _
                                 + LateralNodeCurrentY(front, i, j) * (vGradPsiY(i, j, 1) - rVt * vGradGammaY(i, j, 1) / localN)
    End If
'Rear K dot GradPhi
    localP = rEqP + vGamma(i, j, nNodesZ)
    localN = rEqN + vGamma(i, j, nNodesZ)
    If sSigma(rear, i, j) > 0 Then 'K dot GradPhiP
        rearKdotGrad = LateralNodeCurrentX(rear, i, j) * (vGradPsiX(i, j, nNodesZ) + rVt * vGradGammaX(i, j, nNodesZ) / localP) _
                                + LateralNodeCurrentY(rear, i, j) * (vGradPsiY(i, j, nNodesZ) + rVt * vGradGammaY(i, j, nNodesZ) / localP)
    Else    'K dot GradPhiN
        rearKdotGrad = LateralNodeCurrentX(rear, i, j) * (vGradPsiX(i, j, nNodesZ) - rVt * vGradGammaX(i, j, nNodesZ) / localN) _
                               + LateralNodeCurrentY(rear, i, j) * (vGradPsiY(i, j, nNodesZ) - rVt * vGradGammaY(i, j, nNodesZ) / localN)
    End If
    sPlot(j, i) = sPlot(j, i) - frontKdotGrad - rearKdotGrad
'Contact losses
    contactLoss = 0
    If Not bOpenCircuit Then
        If pType Then
            contactLoss = contactLoss _
                + sContact(front, holes, i, j) * PhiP(sFrontGamma(i, j), sFrontPsi(i, j)) ^ 2 _
                + sContact(front, electrons, i, j) * (PhiN(sFrontGamma(i, j), sFrontPsi(i, j)) + rVoltage) ^ 2
        Else
            contactLoss = contactLoss _
                + sContact(front, holes, i, j) * (PhiP(sFrontGamma(i, j), sFrontPsi(i, j)) - rVoltage) ^ 2 _
                + sContact(front, electrons, i, j) * PhiN(sFrontGamma(i, j), sFrontPsi(i, j)) ^ 2
        End If
        If pType Then
            contactLoss = contactLoss _
                + sContact(rear, holes, i, j) * PhiP(sRearGamma(i, j), sRearPsi(i, j)) ^ 2 _
                + sContact(rear, electrons, i, j) * (PhiN(sRearGamma(i, j), sRearPsi(i, j)) + rVoltage) ^ 2
        Else
            contactLoss = contactLoss _
                + sContact(rear, holes, i, j) * (PhiP(sRearGamma(i, j), sRearPsi(i, j)) - rVoltage) ^ 2 _
                + sContact(rear, electrons, i, j) * PhiN(sRearGamma(i, j), sRearPsi(i, j)) ^ 2
        End If
    End If
    sPlot(j, i) = sPlot(j, i) + contactLoss
Next j: Next i
rPlaneMin = WorksheetFunction.Min(sPlot)
rPlaneMax = WorksheetFunction.max(sPlot)
Call PostData(rPlaneMin, rPlaneMax)
Call NormalizePlot(sPlot, rPlaneMin, rPlaneMax)
Range(normalizedDataCells) = sPlot
Range(plotSurfaceCell).Value = "Front"
Range(plotUnitsCell).Value = "W/cm2"
Call UpdateContourChart
End Sub

Sub ViewPlane(ByRef vArray As Variant, ByRef sArray As Variant)
Dim i As Integer, j As Integer, k As Integer
Dim fraction As Double
If Range(viewCell) = "Top" Then
    k = WorksheetFunction.Floor((nNodesZ - 1) * Range(positionCell) / (10 * deviceHeight), 1) + 1
    fraction = (Range(positionCell) / (10 * deviceHeight) * (nNodesZ - 1) - k + 1)
    If k = nNodesZ Then
        k = nNodesZ - 1: fraction = 1
    End If
    For i = 1 To nNodesX: For j = 1 To nNodesY
        sArray(j, i) = fraction * vArray(i, j, k + 1) + (1 - fraction) * vArray(i, j, k)
    Next j: Next i
End If
If Range(viewCell) = "Face" Then
    j = WorksheetFunction.Floor((nNodesY - 1) * (1 - Range(positionCell) / (10 * deviceLength)), 1) + 1
    fraction = ((1 - Range(positionCell) / (10 * deviceLength)) * (nNodesY - 1) - j + 1)
    If j = nNodesY Then
        j = nNodesY - 1: fraction = 1
    End If
    For i = 1 To nNodesX: For k = 1 To nNodesZ
        sArray(k, i) = fraction * vArray(i, j + 1, k) + (1 - fraction) * vArray(i, j, k)
    Next k: Next i
End If
If Range(viewCell) = "Side" Then
    i = WorksheetFunction.Floor((nNodesX - 1) * Range(positionCell) / (10 * deviceWidth), 1) + 1
    fraction = Range(positionCell) / (10 * deviceWidth) * (nNodesX - 1) - i + 1
    If i = nNodesX Then
        i = nNodesX - 1: fraction = 1
    End If
    For j = 1 To nNodesY: For k = 1 To nNodesZ
        sArray(k, j) = fraction * vArray(i + 1, j, k) + (1 - fraction) * vArray(i, j, k)
    Next k: Next j
End If
End Sub

Sub NormalizePlot(ByRef sArray As Variant, ByVal rMin As Double, ByVal rMax As Double)
Const almostOne = 0.999999      'Avoid having points exactly 1, which exceed the 10-band color range
Dim i As Integer, j As Integer, Imax As Integer, jMax As Integer
Dim iSubdivisions As Integer, jSubdivisions As Integer, m As Integer, n As Integer
Dim localValue As Double, c0 As Double, c1 As Double, c2 As Double, c3 As Double
Dim sNormPlot As Variant
ReDim sNormPlot(1 To nMaxNodes, 1 To nMaxNodes) As Double
'Normalizes sArray and expands it using linear+cross-term interpolation to fill-in the full range for the contour plot.
If Range(viewCell) = "Top" Or Range(viewCell) = "Surface" Then
    Imax = nNodesX: jMax = nNodesY
End If
If Range(viewCell) = "Face" Then
    Imax = nNodesX: jMax = nNodesZ
End If
If Range(viewCell) = "Side" Then
    Imax = nNodesY: jMax = nNodesZ
End If
iSubdivisions = (nMaxNodes - 1) \ (Imax - 1)
jSubdivisions = (nMaxNodes - 1) \ (jMax - 1)
For i = 1 To Imax - 1: For j = 1 To jMax - 1
    c0 = (sArray(j + 1, i + 1) + sArray(j, i + 1) + sArray(j + 1, i) + sArray(j, i)) / 4
    c1 = (sArray(j + 1, i + 1) + sArray(j, i + 1) - sArray(j + 1, i) - sArray(j, i)) / (2 * iSubdivisions)
    c2 = (sArray(j + 1, i + 1) - sArray(j, i + 1) + sArray(j + 1, i) - sArray(j, i)) / (2 * jSubdivisions)
    c3 = (sArray(j + 1, i + 1) - sArray(j, i + 1) - sArray(j + 1, i) + sArray(j, i)) / (iSubdivisions * jSubdivisions)
    For m = 0 To iSubdivisions: For n = 0 To jSubdivisions
        localValue = c0 + c1 * (m - iSubdivisions / 2) + c2 * (n - jSubdivisions / 2) + c3 * (m - iSubdivisions / 2) * (n - jSubdivisions / 2)
        If rMax - rMin > 0 Then
            If localValue = rMax Then
                sNormPlot((j - 1) * jSubdivisions + n + 1, (i - 1) * iSubdivisions + m + 1) = almostOne
            Else
                sNormPlot((j - 1) * jSubdivisions + n + 1, (i - 1) * iSubdivisions + m + 1) = (localValue - rMin) / (rMax - rMin)
            End If
        Else
            If rMax = 0 And rMin = 0 Then
                sNormPlot((j - 1) * jSubdivisions + n + 1, (i - 1) * iSubdivisions + m + 1) = 0
            Else
                sNormPlot((j - 1) * jSubdivisions + n + 1, (i - 1) * iSubdivisions + m + 1) = almostOne
            End If
        End If
    Next n: Next m
Next j: Next i
ReDim sArray(1 To nMaxNodes, 1 To nMaxNodes) As Double
sArray = sNormPlot
End Sub

Sub PlotGeneration()
Dim rVolumeMax As Double, rVolumeMin As Double, rPlaneMax As Double, rPlaneMin As Double
Dim rPlotMax As Double, rPlotMin As Double
Dim i As Integer, j As Integer, k As Integer
If Not bGeneration Then
    If Not bScan And Not bBatch Then MsgBox ("This plot available only after you have Solved the problem.")
    Exit Sub
End If
rVolumeMin = VolumeMin(vGeneration)
rVolumeMax = VolumeMax(vGeneration)
currentPlot = "Generation"
Range(titleCell).Value = currentPlot
Call ValidatePosition(currentPlot)
If Range(viewCell) = "Top" Or Range(viewCell) = "Surface" Then ReDim sPlot(1 To nNodesY, 1 To nNodesX) As Double
If Range(viewCell) = "Face" Then ReDim sPlot(1 To nNodesZ, 1 To nNodesX) As Double
If Range(viewCell) = "Side" Then ReDim sPlot(1 To nNodesZ, 1 To nNodesY) As Double
Call ViewPlane(vGeneration, sPlot)
rPlaneMin = WorksheetFunction.Min(sPlot)
rPlaneMax = WorksheetFunction.max(sPlot)
If bVolumeScale Then rPlotMax = rVolumeMax: rPlotMin = rVolumeMin Else rPlotMax = rPlaneMax: rPlotMin = rPlaneMin
Call PostData(rVolumeMin, rVolumeMax)
Call NormalizePlot(sPlot, rPlotMin, rPlotMax)
Range(normalizedDataCells) = sPlot
Range(plotUnitsCell).Value = "cm-3/s"
Range(lastPlotViewCell).Value = Range(viewCell)
Call UpdateContourChart
End Sub

Sub PlotRecombination()
Dim rVolumeMax As Double, rVolumeMin As Double, rPlaneMax As Double, rPlaneMin As Double
Dim rPlotMax As Double, rPlotMin As Double
Dim i As Integer, j As Integer, k As Integer
If Not bGamma Then
    If Not bScan And Not bBatch Then MsgBox ("This plot available only after you have Solved the problem.")
    Exit Sub
End If
If bGamma And Not bRecombination Then Call CalculateRecombination
rVolumeMin = VolumeMin(vRecombination)
rVolumeMax = VolumeMax(vRecombination)
currentPlot = "Bulk Recomb"
Range(titleCell).Value = currentPlot
Call ValidatePosition(currentPlot)
If Range(viewCell) = "Top" Or Range(viewCell) = "Surface" Then ReDim sPlot(1 To nNodesY, 1 To nNodesX) As Double
If Range(viewCell) = "Face" Then ReDim sPlot(1 To nNodesZ, 1 To nNodesX) As Double
If Range(viewCell) = "Side" Then ReDim sPlot(1 To nNodesZ, 1 To nNodesY) As Double
Call ViewPlane(vRecombination, sPlot)
rPlaneMin = WorksheetFunction.Min(sPlot)
rPlaneMax = WorksheetFunction.max(sPlot)
If bVolumeScale Then rPlotMax = rVolumeMax: rPlotMin = rVolumeMin Else rPlotMax = rPlaneMax: rPlotMin = rPlaneMin
Call PostData(rVolumeMin, rVolumeMax)
Call NormalizePlot(sPlot, rPlotMin, rPlotMax)
Range(normalizedDataCells) = sPlot
Range(plotUnitsCell).Value = "cm-3/s"
Range(lastPlotViewCell).Value = Range(viewCell)
Call UpdateContourChart
End Sub

Sub PlotGamma()
Dim rVolumeMax As Double, rVolumeMin As Double, rPlaneMax As Double, rPlaneMin As Double
Dim rPlotMax As Double, rPlotMin As Double
Dim i As Integer, j As Integer, k As Integer
If Not bGamma Then
    If Not bScan And Not bBatch Then MsgBox ("This plot available only after you have Solved the problem.")
    Exit Sub
End If
rVolumeMin = VolumeMin(vGamma)
rVolumeMax = VolumeMax(vGamma)
currentPlot = "Excess Carriers"
Range(titleCell).Value = currentPlot
Call ValidatePosition(currentPlot)
If Range(viewCell) = "Top" Or Range(viewCell) = "Surface" Then ReDim sPlot(1 To nNodesY, 1 To nNodesX) As Double
If Range(viewCell) = "Face" Then ReDim sPlot(1 To nNodesZ, 1 To nNodesX) As Double
If Range(viewCell) = "Side" Then ReDim sPlot(1 To nNodesZ, 1 To nNodesY) As Double
Call ViewPlane(vGamma, sPlot)
rPlaneMin = WorksheetFunction.Min(sPlot)
rPlaneMax = WorksheetFunction.max(sPlot)
If bVolumeScale Then rPlotMax = rVolumeMax: rPlotMin = rVolumeMin Else rPlotMax = rPlaneMax: rPlotMin = rPlaneMin
Call PostData(rVolumeMin, rVolumeMax)
Call NormalizePlot(sPlot, rPlotMin, rPlotMax)
Range(normalizedDataCells) = sPlot
Range(plotUnitsCell).Value = "cm-3"
Range(lastPlotViewCell).Value = Range(viewCell)
Call UpdateContourChart
End Sub

Sub PlotPsi()
Dim rVolumeMax As Double, rVolumeMin As Double, rPlaneMax As Double, rPlaneMin As Double
Dim rPlotMax As Double, rPlotMin As Double
Dim i As Integer, j As Integer, k As Integer
If Not bGamma Then
    If Not bScan And Not bBatch Then MsgBox ("This plot available only after you have Solved the problem.")
    Exit Sub
End If
If Not bPsi Then Call CalculateRealPsi
rVolumeMin = VolumeMin(vPsi)
rVolumeMax = VolumeMax(vPsi)
currentPlot = "Excess Psi"
Range(titleCell).Value = currentPlot
Call ValidatePosition(currentPlot)
If Range(viewCell) = "Top" Or Range(viewCell) = "Surface" Then ReDim sPlot(1 To nNodesY, 1 To nNodesX) As Double
If Range(viewCell) = "Face" Then ReDim sPlot(1 To nNodesZ, 1 To nNodesX) As Double
If Range(viewCell) = "Side" Then ReDim sPlot(1 To nNodesZ, 1 To nNodesY) As Double
Call ViewPlane(vPsi, sPlot)
rPlaneMin = WorksheetFunction.Min(sPlot)
rPlaneMax = WorksheetFunction.max(sPlot)
If bVolumeScale Then rPlotMax = rVolumeMax: rPlotMin = rVolumeMin Else rPlotMax = rPlaneMax: rPlotMin = rPlaneMin
Call PostData(rVolumeMin, rVolumeMax)
Call NormalizePlot(sPlot, rPlotMin, rPlotMax)
Range(normalizedDataCells) = sPlot
Range(plotUnitsCell).Value = "V"
Range(lastPlotViewCell).Value = Range(viewCell)
Call UpdateContourChart
End Sub

Sub PlotPhi()
Dim rVolumeMax As Double, rVolumeMin As Double, rPlaneMax As Double, rPlaneMin As Double
Dim rPlotMax As Double, rPlotMin As Double
Dim i As Integer, j As Integer, k As Integer
Dim vPhi As Variant
If Not bGamma Then
    If Not bScan And Not bBatch Then MsgBox ("This plot available only after you have Solved the problem.")
    Exit Sub
End If
If Not bPsi Then Call CalculateRealPsi
ReDim vPhi(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
For i = 1 To nNodesX: For j = 1 To nNodesY: For k = 1 To nNodesZ
    If selectQF = "Hole" Then vPhi(i, j, k) = PhiP(vGamma(i, j, k), vPsi(i, j, k))
    If selectQF = "Electron" Then vPhi(i, j, k) = PhiN(vGamma(i, j, k), vPsi(i, j, k))
    If selectQF = "Splitting" Then vPhi(i, j, k) = PhiP(vGamma(i, j, k), vPsi(i, j, k)) - PhiN(vGamma(i, j, k), vPsi(i, j, k))
Next k: Next j: Next i
rVolumeMin = VolumeMin(vPhi)
rVolumeMax = VolumeMax(vPhi)
currentPlot = "Quasi-Fermi"
Range(titleCell).Value = currentPlot
Call ValidatePosition(currentPlot)
If Range(viewCell) = "Top" Or Range(viewCell) = "Surface" Then ReDim sPlot(1 To nNodesY, 1 To nNodesX) As Double
If Range(viewCell) = "Face" Then ReDim sPlot(1 To nNodesZ, 1 To nNodesX) As Double
If Range(viewCell) = "Side" Then ReDim sPlot(1 To nNodesZ, 1 To nNodesY) As Double
Call ViewPlane(vPhi, sPlot)
rPlaneMin = WorksheetFunction.Min(sPlot)
rPlaneMax = WorksheetFunction.max(sPlot)
If bVolumeScale Then rPlotMax = rVolumeMax: rPlotMin = rVolumeMin Else rPlotMax = rPlaneMax: rPlotMin = rPlaneMin
Call PostData(rVolumeMin, rVolumeMax)
Call NormalizePlot(sPlot, rPlotMin, rPlotMax)
Range(normalizedDataCells) = sPlot
Range(plotUnitsCell).Value = "V"
Range(lastPlotViewCell).Value = Range(viewCell)
Call UpdateContourChart
End Sub

Sub PlotCurrent()
Dim rVolumeMax As Double, rVolumeMin As Double, rPlaneMax As Double, rPlaneMin As Double
Dim rPlotMax As Double, rPlotMin As Double
Dim i As Integer, j As Integer, k As Integer
Dim localP As Double, localN As Double
Dim thisJxPdrift As Double, thisJyPdrift As Double, thisJzPdrift As Double
Dim thisJxNdrift As Double, thisJyNdrift As Double, thisJzNdrift As Double
Dim thisJxPdiff As Double, thisJyPdiff As Double, thisJzPdiff As Double
Dim thisJxNdiff As Double, thisJyNdiff As Double, thisJzNdiff As Double
Dim thisDriftX As Double, thisDriftY As Double, thisDriftZ As Double
Dim thisDiffX As Double, thisDiffY As Double, thisDiffZ As Double
Dim thisJx As Double, thisJy As Double, thisJz As Double
Dim Vj As Variant
ReDim Vj(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
If Not bGamma Then
    If Not bScan And Not bBatch Then MsgBox ("This plot available only after you have Solved the problem.")
    Exit Sub
End If
If Not bGradGammaPsi Then Call CalculateGradGammaPsi
For i = 1 To nNodesX: For j = 1 To nNodesY: For k = 1 To nNodesZ
    localP = rEqP + vGamma(i, j, k)
    thisJxPdrift = -q * rDp * localP * vGradPsiX(i, j, k) / rVt
    thisJyPdrift = -q * rDp * localP * vGradPsiY(i, j, k) / rVt
    thisJzPdrift = -q * rDp * localP * vGradPsiZ(i, j, k) / rVt
    thisJxPdiff = -q * rDp * vGradGammaX(i, j, k)
    thisJyPdiff = -q * rDp * vGradGammaY(i, j, k)
    thisJzPdiff = -q * rDp * vGradGammaZ(i, j, k)
    localN = rEqN + vGamma(i, j, k)
    thisJxNdrift = -q * rDn * localN * vGradPsiX(i, j, k) / rVt
    thisJyNdrift = -q * rDn * localN * vGradPsiY(i, j, k) / rVt
    thisJzNdrift = -q * rDn * localN * vGradPsiZ(i, j, k) / rVt
    thisJxNdiff = q * rDn * vGradGammaX(i, j, k)
    thisJyNdiff = q * rDn * vGradGammaY(i, j, k)
    thisJzNdiff = q * rDn * vGradGammaZ(i, j, k)
    'Test selections to determine which components to include in plotted current density, starting with carrier type
    If selectJCarrier = "Hole" Then
        thisDriftX = thisJxPdrift: thisDriftY = thisJyPdrift: thisDriftZ = thisJzPdrift
        thisDiffX = thisJxPdiff: thisDiffY = thisJyPdiff: thisDiffZ = thisJzPdiff
    End If
    If selectJCarrier = "Electron" Then
        thisDriftX = thisJxNdrift: thisDriftY = thisJyNdrift: thisDriftZ = thisJzNdrift
        thisDiffX = thisJxNdiff: thisDiffY = thisJyNdiff: thisDiffZ = thisJzNdiff
    End If
    If selectJCarrier = "Total" Then
        thisDriftX = thisJxPdrift + thisJxNdrift: thisDriftY = thisJyPdrift + thisJyNdrift: thisDriftZ = thisJzPdrift + thisJzNdrift
        thisDiffX = thisJxPdiff + thisJxNdiff: thisDiffY = thisJyPdiff + thisJyNdiff: thisDiffZ = thisJzPdiff + thisJzNdiff
    End If
    'Now based on portion
    If selectJPortion = "Drift" Then _
        thisJx = thisDriftX: thisJy = thisDriftY: thisJz = thisDriftZ
    If selectJPortion = "Diffuse" Then _
        thisJx = thisDiffX: thisJy = thisDiffY: thisJz = thisDiffZ
    If selectJPortion = "Total" Then _
        thisJx = thisDriftX + thisDiffX: thisJy = thisDriftY + thisDiffY: thisJz = thisDriftZ + thisDiffZ
    'Finally, based on direction
    If selectJDirection = "X-axis" Then Vj(i, j, k) = thisJx
    If selectJDirection = "Y-axis" Then Vj(i, j, k) = thisJy
    If selectJDirection = "Z-axis" Then Vj(i, j, k) = thisJz
    If selectJDirection = "Total" Then Vj(i, j, k) = Sqr(thisJx ^ 2 + thisJy ^ 2 + thisJz ^ 2)
Next k: Next j: Next i
rVolumeMin = VolumeMin(Vj)
rVolumeMax = VolumeMax(Vj)
currentPlot = "Bulk Current"
Range(titleCell).Value = currentPlot
Call ValidatePosition(currentPlot)
If Range(viewCell) = "Top" Or Range(viewCell) = "Surface" Then ReDim sPlot(1 To nNodesY, 1 To nNodesX) As Double
If Range(viewCell) = "Face" Then ReDim sPlot(1 To nNodesZ, 1 To nNodesX) As Double
If Range(viewCell) = "Side" Then ReDim sPlot(1 To nNodesZ, 1 To nNodesY) As Double
Call ViewPlane(Vj, sPlot)
rPlaneMin = WorksheetFunction.Min(sPlot)
rPlaneMax = WorksheetFunction.max(sPlot)
If bVolumeScale Then rPlotMax = rVolumeMax: rPlotMin = rVolumeMin Else rPlotMax = rPlaneMax: rPlotMin = rPlaneMin
Call PostData(rVolumeMin, rVolumeMax)
Call NormalizePlot(sPlot, rPlotMin, rPlotMax)
Range(normalizedDataCells) = sPlot
Range(plotUnitsCell).Value = "A/cm2"
Range(lastPlotViewCell).Value = Range(viewCell)
Call UpdateContourChart
End Sub

Sub CreateImage()
Const initialOffset = 30                    'Offset the image right and down from the currently selected cell
Const offsetLeft = -12, offsetTop = -12     'These offsets are introduced by Excel copy/paste
Dim plotWidth As Double, plotHeight As Double
Dim chartWidth As Variant, chartHeight As Variant
Dim aspectPicture As Double, aspectDevice As Double

Call MapInputParameters     'Necessary to set dimensions in case user images layout prior to a solution
chartWidth = Sheets("Dashboard").ChartObjects(contourPlot).Width
chartHeight = Sheets("Dashboard").ChartObjects(contourPlot).Height
Sheets("Dashboard").ChartObjects(contourPlot).Chart.PlotArea.Width = chartWidth     'Make sure the plot fills the chart
Sheets("Dashboard").ChartObjects(contourPlot).Chart.PlotArea.Height = chartHeight
If chartWidth > chartHeight Then   'Excel fits the square plot into the rectangular region with a small minimum border
    plotHeight = chartHeight - 8: plotWidth = plotHeight
Else
    plotWidth = chartWidth - 8: plotHeight = plotWidth
End If
Sheets("Dashboard").ChartObjects(contourPlot).Copy
Sheets("Image").Select
ActiveWindow.Zoom = 100
ActiveSheet.Pictures.Paste.Select
Selection.ShapeRange.IncrementLeft initialOffset
Selection.ShapeRange.IncrementTop initialOffset

Selection.ShapeRange.PictureFormat.cropLeft = (chartWidth - plotWidth) / 2
Selection.ShapeRange.PictureFormat.cropRight = (chartWidth - plotWidth) / 2
Selection.ShapeRange.PictureFormat.cropTop = (chartHeight - plotHeight) / 2
Selection.ShapeRange.PictureFormat.cropBottom = (chartHeight - plotHeight) / 2
Selection.Copy
    
'First Row
ActiveSheet.Paste
Selection.ShapeRange.IncrementLeft plotWidth + offsetLeft
Selection.ShapeRange.IncrementTop offsetTop
Selection.ShapeRange.Flip msoFlipHorizontal
ActiveSheet.Paste
Selection.ShapeRange.IncrementLeft plotWidth + offsetLeft
Selection.ShapeRange.IncrementTop offsetTop
ActiveSheet.Paste
Selection.ShapeRange.IncrementLeft plotWidth + offsetLeft
Selection.ShapeRange.IncrementTop offsetTop
Selection.ShapeRange.Flip msoFlipHorizontal
If Range(viewCell) = "Top" Or Range(viewCell) = "Surface" Then
    'Second Row
    ActiveSheet.Paste
    Selection.ShapeRange.IncrementLeft -3 * plotWidth + offsetLeft
    Selection.ShapeRange.IncrementTop plotHeight + offsetTop
    Selection.ShapeRange.Flip msoFlipVertical
    ActiveSheet.Paste
    Selection.ShapeRange.IncrementLeft plotWidth + offsetLeft
    Selection.ShapeRange.IncrementTop offsetTop
    Selection.ShapeRange.Flip msoFlipHorizontal
    Selection.ShapeRange.Flip msoFlipVertical
    ActiveSheet.Paste
    Selection.ShapeRange.IncrementLeft plotWidth + offsetLeft
    Selection.ShapeRange.IncrementTop offsetTop
    Selection.ShapeRange.Flip msoFlipVertical
    ActiveSheet.Paste
    Selection.ShapeRange.IncrementLeft plotWidth + offsetLeft
    Selection.ShapeRange.IncrementTop offsetTop
    Selection.ShapeRange.Flip msoFlipHorizontal
    Selection.ShapeRange.Flip msoFlipVertical
End If
ActiveSheet.Pictures.Select
Selection.ShapeRange.Group.Select
'Adjust aspect ratio of image to match actual device
If Range(viewCell) = "Top" Or Range(viewCell) = "Surface" Then
    aspectPicture = 2 * Selection.ShapeRange.Height / Selection.ShapeRange.Width
    aspectDevice = deviceLength / deviceWidth
    If deviceWidth >= deviceLength Then
        Selection.ShapeRange.ScaleHeight aspectDevice / aspectPicture, msoFalse
    Else
        Selection.ShapeRange.ScaleWidth aspectPicture / aspectDevice, msoFalse
    End If
End If
If Range(viewCell) = "Face" Then
    aspectPicture = 4 * Selection.ShapeRange.Height / Selection.ShapeRange.Width
    aspectDevice = deviceHeight / deviceWidth
    If deviceWidth >= deviceHeight Then
        Selection.ShapeRange.ScaleHeight aspectDevice / aspectPicture, msoFalse
    End If
End If
If Range(viewCell) = "Side" Then
    aspectPicture = 4 * Selection.ShapeRange.Height / Selection.ShapeRange.Width
    aspectDevice = deviceHeight / deviceLength
    If deviceLength >= deviceHeight Then
        Selection.ShapeRange.ScaleHeight aspectDevice / aspectPicture, msoFalse
    End If
End If
End Sub
