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
Dim i As Integer, j As Integer, k As Integer
Call MapInputParameters
Select Case currentPlot
    Case "Doping": Call PlotDoping
    Case "Carriers": Call PlotCarriers
    Case "Generation": Call PlotGeneration
    Case "Recombination": Call PlotRecombination
    Case "Potential": Call PlotPotential
    Case "Charge": Call PlotCharge
    Case "Quasi-Fermi": Call PlotPhi
    Case "Current": Call PlotCurrent
    Case "Electric Field": Call PlotField
    Case "Velocity": Call PlotVelocity
    Case "Mobility": Call PlotMobility
    Case "Qgamma": Call PlotQgamma
    Case "Gamma": Call PlotGamma
    Case "Texture": Call PlotTexture
    Case "Qcharge": Call PlotQcharge
    Case "SRV": Call PlotSRV
    Case "SurfaceRecomb": Call PlotSurfaceRecomb
End Select
End Sub

Function DirectionalPlot() As Boolean
DirectionalPlot = currentPlot = "Current" Or currentPlot = "Velocity" Or currentPlot = "Electric Field" Or currentPlot = "GradGamma"
End Function

Function CarrierPlot() As Boolean
CarrierPlot = currentPlot = "Quasi-Fermi" Or currentPlot = "Current" Or currentPlot = "Carriers" Or currentPlot = "Velocity" Or currentPlot = "Mobility" Or currentPlot = "Qgamma"
End Function

Function BipolarPlot() As Boolean
BipolarPlot = currentPlot = "Doping" Or currentPlot = "Charge" Or currentPlot = "Qcharge" Or (currentPlot = "Carriers" And nSelectCarrier = combined)
End Function

Sub UpdateContourChart()
'Changing the legend colors forces Excel to refresh the chart
Dim Colors As Variant
Call MaskVoid
Call DisplayArrows(1)
'Default green scheme
    Colors = Array(RGB(72, 114, 44), RGB(82, 129, 50), RGB(90, 141, 56), RGB(98, 151, 62), RGB(104, 161, 66), _
                             RGB(112, 171, 70), RGB(137, 183, 112), RGB(159, 195, 143), RGB(179, 205, 167), RGB(193, 215, 185))
If currentPlot = "Generation" Then     'yellow scheme
    Colors = Array(RGB(80, 80, 30), RGB(100, 100, 0), RGB(125, 125, 0), RGB(150, 150, 0), RGB(175, 175, 0), _
                             RGB(200, 200, 0), RGB(225, 225, 0), RGB(250, 250, 0), RGB(255, 255, 100), RGB(255, 255, 150))
End If
If CarrierPlot And nSelectCarrier = holes Then                       'blue scheme
    Colors = Array(RGB(58, 102, 141), RGB(66, 116, 159), RGB(74, 126, 175), RGB(80, 135, 187), RGB(86, 145, 199), _
                             RGB(90, 153, 211), RGB(124, 169, 217), RGB(149, 183, 223), RGB(171, 197, 227), RGB(189, 207, 233))
End If
If CarrierPlot And nSelectCarrier = electrons Then                       'orange scheme
    Colors = Array(RGB(171, 128, 0), RGB(191, 143, 0), RGB(209, 157, 0), RGB(225, 169, 0), RGB(239, 179, 0), _
                             RGB(255, 191, 0), RGB(255, 199, 92), RGB(255, 207, 131), RGB(255, 215, 159), RGB(255, 223, 179))
End If
If BipolarPlot Then   'Bipolar scheme
    Colors = Array(RGB(255, 191, 0), RGB(255, 199, 92), RGB(255, 207, 131), RGB(255, 215, 159), RGB(230, 210, 180), _
                             RGB(205, 210, 220), RGB(171, 197, 227), RGB(149, 183, 223), RGB(124, 169, 217), RGB(90, 153, 211))
End If
If currentPlot = "Potential" Then  'Inverted Bipolar scheme
    Colors = Array(RGB(90, 153, 211), RGB(124, 169, 217), RGB(149, 183, 223), RGB(171, 197, 227), RGB(205, 210, 220), _
                             RGB(230, 210, 180), RGB(255, 215, 159), RGB(255, 207, 131), RGB(255, 199, 92), RGB(255, 191, 0))
End If
If bConvergenceFailure Then 'red scheme
    Colors = Array(RGB(157, 82, 28), RGB(177, 92, 34), RGB(195, 102, 38), RGB(209, 110, 42), RGB(223, 118, 46), _
                             RGB(235, 124, 48), RGB(237, 145, 104), RGB(239, 165, 137), RGB(243, 183, 163), RGB(243, 197, 183))
End If
If bIntermediatePlot Then   'gray scheme
    Colors = Array(RGB(108, 108, 108), RGB(122, 122, 122), RGB(135, 135, 135), RGB(145, 145, 145), RGB(155, 155, 155), _
                             RGB(163, 163, 163), RGB(177, 177, 177), RGB(181, 181, 181), RGB(201, 201, 201), RGB(211, 211, 211))
End If
Worksheets("Dashboard").ChartObjects(contourPlot).Chart.SetElement (msoElementLegendBottom)
With Worksheets("Dashboard").ChartObjects(contourPlot).Chart.Legend
    .LegendEntries(1).LegendKey.Interior.Color = Colors(0)
    .LegendEntries(2).LegendKey.Interior.Color = Colors(1)
    .LegendEntries(3).LegendKey.Interior.Color = Colors(2)
    .LegendEntries(4).LegendKey.Interior.Color = Colors(3)
    .LegendEntries(5).LegendKey.Interior.Color = Colors(4)
    .LegendEntries(6).LegendKey.Interior.Color = Colors(5)
    .LegendEntries(7).LegendKey.Interior.Color = Colors(6)
    .LegendEntries(8).LegendKey.Interior.Color = Colors(7)
    .LegendEntries(9).LegendKey.Interior.Color = Colors(8)
    .LegendEntries(10).LegendKey.Interior.Color = Colors(9)
End With
Worksheets("Dashboard").ChartObjects(contourPlot).Chart.Legend.Delete
Call RefreshCharts
End Sub

Sub GetPlotLocation(ByRef rLeft As Single, ByRef rTop As Single, ByRef rWidth As Single, ByRef rHeight As Single)
rWidth = Sheets("Dashboard").ChartObjects(contourPlot).Width - 6
rHeight = Sheets("Dashboard").ChartObjects(contourPlot).Height - 6
If rWidth > rHeight Then
    rLeft = Sheets("Dashboard").ChartObjects(contourPlot).left + 3 + (rWidth - rHeight) / 2
    rTop = Sheets("Dashboard").ChartObjects(contourPlot).top + 3
    rWidth = rHeight
Else
    rLeft = Sheets("Dashboard").ChartObjects(contourPlot).left + 3
    rTop = Sheets("Dashboard").ChartObjects(contourPlot).top + 3 + (rHeight - rWidth) / 2
    rHeight = rWidth
End If
End Sub

Sub ValidatePosition(ByVal thisPlot As String)
Dim maxPosition As Double
bEnableChangeHandlers = False
maxPosition = 0
'Note that displayed dimensions are mostly in um, but stored dimensions are in cm for equation solver
Select Case Range(viewCell)
    Case "Surface": maxPosition = 10000# * textureDepth
    Case "Top": maxPosition = 10000# * deviceHeight
    Case "Face": maxPosition = 10000# * deviceLength
    Case "Side": maxPosition = 10000# * deviceWidth
End Select
If Range(positionCell) < 0 Or Range(positionCell) = "" Or Not IsNumeric(Range(positionCell)) Then Range(positionCell).Value = 0
If Range(positionCell) > maxPosition Then Range(positionCell).Value = maxPosition
bEnableChangeHandlers = True
End Sub

Sub ScanPlot()
Dim i As Integer, j As Integer, k As Integer, n As Integer, nScanSteps As Integer
Dim rEndTime As Single
Dim rPosition As Double, maxPosition As Double     'um
Dim saveView As String
If bNeedReset Then Call ResetAll
rPosition = Range(positionCell)       'um
maxPosition = 0
'Note that displayed dimensions are mostly in um, but stored dimensions are in cm for equation solver
Select Case Range(viewCell)
    Case "Surface": maxPosition = 10000# * textureDepth
    Case "Top": maxPosition = 10000# * deviceHeight
    Case "Face": maxPosition = 10000# * deviceLength
    Case "Side": maxPosition = 10000# * deviceWidth
End Select
nScanSteps = nPlotNodes - 1
If nDimensions = 1 Or (nDimensions = 2 And Range(viewCell) = "Face") Or Range(viewCell) = "Surface" Then nScanSteps = 1
bVolumeScale = True
rEndTime = Timer + 1    'Wait at zero position with revised volume scaling before starting scan
Range(positionCell).Value = 0: DoEvents 'Triggers OnPlotChange
RefreshCharts
Do While Timer < rEndTime: Loop
saveView = Range(viewCell)
If Range(viewCell) = "Surface" Then Range(viewCell).Value = "Top"
For k = 1 To nScanSteps
    rEndTime = Timer + 0.1  'Don't update faster than 10 frames per second
    Range(positionCell).Value = Round(k * maxPosition / nScanSteps, 3): DoEvents 'Triggers OnPlotChange
    If bMac Then RefreshCharts      'Nothing additional needed here for Windows
    Do While Timer < rEndTime: Loop
Next k
rEndTime = Timer + 1    'Wait before restoring pre-scan position
Do While Timer < rEndTime: Loop
Range(viewCell).Value = saveView
bVolumeScale = False
Range(positionCell).Value = rPosition: DoEvents   'Triggers OnPlotChange
RefreshCharts
End Sub

Sub PostData(ByRef sArray As Variant, ByVal rMin As Double, ByVal rMax As Double)
'Posts the current contents of sArray in columnar format on the Data! sheet and enters abscissa and ordinate values for plot data
'Posts the min and max values for the current view plane above the contour plot
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
Range(plotCarrierCell).Value = ""
Range(plotDirectionCell).Value = ""
Range(tabularUnitsCell).Value = "µm"
Select Case Range(viewCell)
    Case "Surface"
        maxAbscissa = 10000# * deviceWidth
        maxOrdinate = 10000# * deviceLength
        columnHeader(1, 1) = "X(µm)"
        columnHeader(1, 2) = "Y(µm)"
        nAbscissaEntries = nNodesX
        nOrdinateEntries = nNodesY
    Case "Top"
        maxAbscissa = 10000# * deviceWidth
        maxOrdinate = 10000# * deviceLength
        columnHeader(1, 1) = "X(µm)"
        columnHeader(1, 2) = "Y(µm)"
        nAbscissaEntries = nNodesX
        nOrdinateEntries = nNodesY
    Case "Face"
        maxAbscissa = 10000# * deviceWidth
        maxOrdinate = 10000# * deviceHeight
        columnHeader(1, 1) = "X(µm)"
        columnHeader(1, 2) = "Z(µm)"
        nAbscissaEntries = nNodesX
        nOrdinateEntries = nNodesZ
    Case "Side"
        maxAbscissa = 10000# * deviceLength
        maxOrdinate = 10000# * deviceHeight
        columnHeader(1, 1) = "Y(µm)"
        columnHeader(1, 2) = "Z(µm)"
        nAbscissaEntries = nNodesY
        nOrdinateEntries = nNodesZ
End Select
Range(columnarHeaderCells) = columnHeader
'Fill abscissa and ordinate fields for tabular data
For i = 1 To nPlotNodes
    abscissaData(1, i) = maxAbscissa * (i - 1) / (nPlotNodes - 1)
Next i
Range(tabularAbscissaCells) = abscissaData
For j = 1 To nPlotHarmonics + 1
    ordinateData(j, 1) = maxOrdinate * (j - 1) / (nPlotNodes - 1)
Next j
Range(tabularOrdinateCells) = ordinateData
'Fill columnar data fields
row = 0
For i = 1 To nAbscissaEntries: For j = 1 To nOrdinateEntries
    row = row + 1
    columnData(row, 1) = maxAbscissa * (i - 1) / (nAbscissaEntries - 1)
    columnData(row, 2) = maxOrdinate * (j - 1) / (nOrdinateEntries - 1)
    columnData(row, 3) = sArray(j, i)
Next j: Next i
Range(columnarDataCells) = columnData
'Fill columnar metadata fields
Range(plotNameCell).Value = currentPlot
Range(plotViewCell).Value = Range(viewCell)
Range(plotPositionCell).Value = Range(positionCell)
Range(plotMinCell).Value = rMin
Range(plotMaxCell).Value = rMax
Range(plotResolutionCell).Value = Range(resolutionCell)
Range(minCell) = WorksheetFunction.Min(sArray)
Range(maxCell) = WorksheetFunction.max(sArray)
Range(unitCell).Value = Range(plotUnitsCell)
If currentPlot = "Quasi-Fermi" Or currentPlot = "Current" Or currentPlot = "Velocity" Or currentPlot = "Gamma" Then
    Select Case nSelectCarrier
        Case holes: Range(plotCarrierCell).Value = "Holes"
        Case electrons: Range(plotCarrierCell).Value = "Electrons"
        Case combined: Range(plotCarrierCell).Value = "Combined"
    End Select
End If
End Sub

Sub ViewPlane(ByRef vArray As Variant, ByRef sArray As Variant, ByRef rMin As Double, ByRef rMax As Double)
'Linear interpolation for view positions that lie between solution planes
'Surface view is a special case that returns values at the texture-etched top surface
'Returns the min and max values appropriate for plot normalization (volume limits except for Surface view)
Dim i As Integer, j As Integer, k As Integer
Dim fraction As Double
Select Case Range(viewCell)
    Case "Surface"
        ReDim sArray(1 To nNodesY, 1 To nNodesX) As Double
        For i = 1 To nNodesX: For j = 1 To nNodesY
            sArray(j, i) = vArray(i, j, sTexture(i, j))
        Next j: Next i
    Case "Top"
        ReDim sArray(1 To nNodesY, 1 To nNodesX) As Double
        k = WorksheetFunction.Floor((nNodesZ - 1) * Range(positionCell) / (10000# * deviceHeight), 1) + 1
        fraction = (Range(positionCell) / (10000# * deviceHeight) * (nNodesZ - 1) - k + 1)
        If k = nNodesZ Then
            k = nNodesZ - 1: fraction = 1
        End If
        For i = 1 To nNodesX: For j = 1 To nNodesY
            sArray(j, i) = fraction * vArray(i, j, k + 1) + (1 - fraction) * vArray(i, j, k)
        Next j: Next i
    Case "Face"
        ReDim sArray(1 To nNodesZ, 1 To nNodesX) As Double
        j = WorksheetFunction.Floor((nNodesY - 1) * (1 - Range(positionCell) / (10000# * deviceLength)), 1) + 1
        fraction = ((1 - Range(positionCell) / (10000# * deviceLength)) * (nNodesY - 1) - j + 1)
        If j = nNodesY Then
            j = nNodesY - 1: fraction = 1
        End If
        For i = 1 To nNodesX: For k = 1 To nNodesZ
            sArray(k, i) = fraction * vArray(i, j + 1, k) + (1 - fraction) * vArray(i, j, k)
        Next k: Next i
    Case "Side"
        ReDim sArray(1 To nNodesZ, 1 To nNodesY) As Double
        i = WorksheetFunction.Floor((nNodesX - 1) * Range(positionCell) / (10000# * deviceWidth), 1) + 1
        fraction = Range(positionCell) / (10000# * deviceWidth) * (nNodesX - 1) - i + 1
        If i = nNodesX Then
            i = nNodesX - 1: fraction = 1
        End If
        For j = 1 To nNodesY: For k = 1 To nNodesZ
            sArray(k, j) = fraction * vArray(i + 1, j, k) + (1 - fraction) * vArray(i, j, k)
        Next k: Next j
End Select
If Range(viewCell) = "Surface" Then
    rMin = WorksheetFunction.Min(sArray): rMax = WorksheetFunction.max(sArray)
Else
    rMin = VolumeMin(vArray): rMax = VolumeMax(vArray)
End If
End Sub

Sub NormalizePlot(ByRef sArray As Variant, ByVal rMin As Double, ByVal rMax As Double)
'Adjust size of sArray for tabular data using linear (triangular functional) segmentation to match plot dimensions
'Normalizes plot for graphic display
Const almostOne = 0.99      'Avoid having normalized points exactly 1, which exceed the 10-band color range of the normalized plot
Const almostZero = 0.01     'Assign normalized points close to zero, equal to zero, to reduce spurious noise in normalized plot
Dim i As Integer, j As Integer
Dim sTransposeArray As Variant, sDataPlot As Variant, sNormPlot As Variant
ReDim sDataPlot(1 To nPlotNodes, 1 To nPlotNodes) As Double
ReDim sNormPlot(1 To nPlotNodes, 1 To nPlotNodes) As Double
'Convert sArray to internal row=x, column=y format. This rather cumbersome implementation is necessary for Mac compatibility
Select Case Range(viewCell)
    Case "Surface"
        ReDim sTransposeArray(1 To nNodesX, 1 To nNodesY) As Double
        sTransposeArray = Application.Transpose(sArray)
        sDataPlot = Application.Transpose(Application.MMult(r2tY, Application.Transpose(Application.MMult(r2tX, sTransposeArray))))
    Case "Top"
        ReDim sTransposeArray(1 To nNodesX, 1 To nNodesY) As Double
        sTransposeArray = Application.Transpose(sArray)
        sDataPlot = Application.Transpose(Application.MMult(r2tY, Application.Transpose(Application.MMult(r2tX, sTransposeArray))))
    Case "Face"
        ReDim sTransposeArray(1 To nNodesX, 1 To nNodesZ) As Double
        sTransposeArray = Application.Transpose(sArray)
        sDataPlot = Application.Transpose(Application.MMult(r2tZ, Application.Transpose(Application.MMult(r2tX, sTransposeArray))))
    Case "Side"
        ReDim sTransposeArray(1 To nNodesY, 1 To nNodesZ) As Double
        sTransposeArray = Application.Transpose(sArray)
        sDataPlot = Application.Transpose(Application.MMult(r2tZ, Application.Transpose(Application.MMult(r2tY, sTransposeArray))))
End Select
Range(tabularDataCells) = Application.Transpose(sDataPlot)
sNormPlot = sDataPlot
For i = 1 To nPlotNodes: For j = 1 To nPlotNodes
    If rMax > rMin Then
        If sNormPlot(i, j) > rMin + almostOne * (rMax - rMin) Then sNormPlot(i, j) = almostOne Else sNormPlot(i, j) = (sNormPlot(i, j) - rMin) / (rMax - rMin)
        If sNormPlot(i, j) < almostZero Then sNormPlot(i, j) = 0
    Else
        If rMax = 0 And rMin = 0 Then sNormPlot(i, j) = 0 Else sNormPlot(i, j) = almostOne
    End If
Next j: Next i
Range(normalizedDataCells) = Application.Transpose(sNormPlot)
End Sub

'*** VOLUMETRIC PLOTS ***

Sub PlotDoping()
Dim rMax As Double, rMin As Double
Dim i As Integer, j As Integer, k As Integer
Dim vDoping As Variant
If bNeedReset Then Call ResetAll
ReDim vDoping(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
For i = 1 To nNodesX: For j = 1 To nNodesY: For k = 1 To nNodesZ
    vDoping(i, j, k) = vDopingP(i, j, k) - vDopingN(i, j, k)
Next k: Next j: Next i
For i = 1 To nNodesX: For j = 1 To nNodesY: For k = 1 To sTexture(i, j) - 1
    vDoping(i, j, k) = vDoping(i, j, sTexture(i, j))
Next k: Next j: Next i
currentPlot = "Doping"
Range(titleCell).Value = currentPlot
Call ValidatePosition(currentPlot)
Call ViewPlane(vDoping, sPlot, rMin, rMax)
Call PostData(sPlot, rMin, rMax)
If -rMin > rMax Then rMax = -rMin
If rMax > -rMin Then rMin = -rMax
Call NormalizePlot(sPlot, rMin, rMax)
Range(plotUnitsCell).Value = "cm-3"
Range(lastPlotViewCell).Value = Range(viewCell)
Call UpdateContourChart
End Sub

Sub PlotMobility()
Dim rMax As Double, rMin As Double
Dim i As Integer, j As Integer, k As Integer
Dim vMobility As Variant
If bNeedReset Then Call ResetAll
ReDim vMobility(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
For i = 1 To nNodesX: For j = 1 To nNodesY: For k = 1 To nNodesZ
    Select Case nSelectCarrier
        Case holes: vMobility(i, j, k) = vDp(i, j, k) / rVt
        Case electrons: vMobility(i, j, k) = vDn(i, j, k) / rVt
        Case combined: vMobility(i, j, k) = 2 * vDp(i, j, k) * vDn(i, j, k) / (vDp(i, j, k) + vDn(i, j, k)) / rVt       'Ambipolar
    End Select
Next k: Next j: Next i
For i = 1 To nNodesX: For j = 1 To nNodesY: For k = 1 To sTexture(i, j) - 1
    vMobility(i, j, k) = vMobility(i, j, sTexture(i, j))
Next k: Next j: Next i
currentPlot = "Mobility"
Range(titleCell).Value = currentPlot
Call ValidatePosition(currentPlot)
Call ViewPlane(vMobility, sPlot, rMin, rMax)
Call PostData(sPlot, rMin, rMax)
Call NormalizePlot(sPlot, rMin, rMax)
Range(plotUnitsCell).Value = "cm2/Vs"
Range(lastPlotViewCell).Value = Range(viewCell)
Call UpdateContourChart
End Sub

Sub PlotCarriers()
Dim rMax As Double, rMin As Double
Dim i As Integer, j As Integer, k As Integer
Dim vCarrier As Variant
If bNeedReset Then Call ResetAll
ReDim vCarrier(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
For i = 1 To nNodesX: For j = 1 To nNodesY: For k = 1 To nNodesZ
    Select Case nSelectCarrier
        Case holes:  vCarrier(i, j, k) = rNi * ExpPower((vPhiP(i, j, k) - vPsi(i, j, k) + vVp(i, j, k)) / rVt)
        Case electrons: vCarrier(i, j, k) = rNi * ExpPower((vPsi(i, j, k) - vPhiN(i, j, k) + vVn(i, j, k)) / rVt)
        Case combined: vCarrier(i, j, k) = rNi * ExpPower((vPhiP(i, j, k) - vPsi(i, j, k) + vVp(i, j, k)) / rVt) - rNi * ExpPower((vPsi(i, j, k) - vPhiN(i, j, k) + vVn(i, j, k)) / rVt)
    End Select
Next k: Next j: Next i
For i = 1 To nNodesX: For j = 1 To nNodesY: For k = 1 To sTexture(i, j) - 1
    vCarrier(i, j, k) = vCarrier(i, j, sTexture(i, j))
Next k: Next j: Next i
currentPlot = "Carriers"
Range(titleCell).Value = currentPlot
Call ValidatePosition(currentPlot)
Call ViewPlane(vCarrier, sPlot, rMin, rMax)
Call PostData(sPlot, rMin, rMax)
If nSelectCarrier = combined Then
    If -rMin > rMax Then rMax = -rMin
    If rMax > -rMin Then rMin = -rMax
End If
Call NormalizePlot(sPlot, rMin, rMax)
Range(plotUnitsCell).Value = "cm-3"
Range(lastPlotViewCell).Value = Range(viewCell)
Call UpdateContourChart
End Sub

Sub PlotGeneration()
Dim rMax As Double, rMin As Double
Dim i As Integer, j As Integer, k As Integer
Dim vG As Variant
If bNeedReset Then Call ResetAll
ReDim vG(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
vG = vGeneration
For i = 1 To nNodesX: For j = 1 To nNodesY: For k = 1 To sTexture(i, j) - 1
    vG(i, j, k) = vG(i, j, sTexture(i, j))
Next k: Next j: Next i
currentPlot = "Generation"
Range(titleCell).Value = currentPlot
Call ValidatePosition(currentPlot)
Call ViewPlane(vG, sPlot, rMin, rMax)
Call PostData(sPlot, rMin, rMax)
Call NormalizePlot(sPlot, rMin, rMax)
Range(plotUnitsCell).Value = "cm-3/s"
Range(lastPlotViewCell).Value = Range(viewCell)
Call UpdateContourChart
End Sub

Sub PlotRecombination()
Dim rMax As Double, rMin As Double
Dim i As Integer, j As Integer, k As Integer
Dim vR As Variant
If bNeedReset Then Call ResetAll
ReDim vR(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
vR = vRecombination
For i = 1 To nNodesX: For j = 1 To nNodesY: For k = 1 To sTexture(i, j) - 1
    vR(i, j, k) = vR(i, j, sTexture(i, j))
Next k: Next j: Next i
currentPlot = "Recombination"
Range(titleCell).Value = currentPlot
Call ValidatePosition(currentPlot)
Call ViewPlane(vR, sPlot, rMin, rMax)
Call PostData(sPlot, rMin, rMax)
Call NormalizePlot(sPlot, rMin, rMax)
Range(plotUnitsCell).Value = "cm-3/s"
Range(lastPlotViewCell).Value = Range(viewCell)
Call UpdateContourChart
End Sub

Sub PlotPotential()
Dim rMax As Double, rMin As Double
Dim i As Integer, j As Integer, k As Integer
Dim vP As Variant
If bNeedReset Then Call ResetAll
ReDim vP(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
vP = vPsi
For i = 1 To nNodesX: For j = 1 To nNodesY: For k = 1 To sTexture(i, j) - 1
    vP(i, j, k) = vP(i, j, sTexture(i, j))
Next k: Next j: Next i
currentPlot = "Potential"
Range(titleCell).Value = currentPlot
Call ValidatePosition(currentPlot)
Call ViewPlane(vP, sPlot, rMin, rMax)
Call PostData(sPlot, rMin, rMax)
If -rMin > rMax Then rMax = -rMin
If rMax > -rMin Then rMin = -rMax
Call NormalizePlot(sPlot, rMin, rMax)
Range(plotUnitsCell).Value = "V"
Range(lastPlotViewCell).Value = Range(viewCell)
Call UpdateContourChart
End Sub

Sub PlotCharge()
Dim rMax As Double, rMin As Double
Dim i As Integer, j As Integer, k As Integer
Dim localP As Double, localN As Double
Dim vCharge As Variant
If bNeedReset Then Call ResetAll
ReDim vCharge(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
For i = 1 To nNodesX: For j = 1 To nNodesY: For k = 1 To nNodesZ
    localP = rNi * ExpPower((vPhiP(i, j, k) - vPsi(i, j, k) + vVp(i, j, k)) / rVt)
    localN = rNi * ExpPower((vPsi(i, j, k) - vPhiN(i, j, k) + vVn(i, j, k)) / rVt)
    vCharge(i, j, k) = localP - localN + vDopingN(i, j, k) - vDopingP(i, j, k)
Next k: Next j: Next i
For i = 1 To nNodesX: For j = 1 To nNodesY: For k = 1 To sTexture(i, j) - 1
    vCharge(i, j, k) = vCharge(i, j, sTexture(i, j))
Next k: Next j: Next i
currentPlot = "Charge"
Range(titleCell).Value = currentPlot
Call ValidatePosition(currentPlot)
Call ViewPlane(vCharge, sPlot, rMin, rMax)
Call PostData(sPlot, rMin, rMax)
If -rMin > rMax Then rMax = -rMin
If rMax > -rMin Then rMin = -rMax
Call NormalizePlot(sPlot, rMin, rMax)
Range(plotUnitsCell).Value = "cm-3"
Range(lastPlotViewCell).Value = Range(viewCell)
Call UpdateContourChart
End Sub

Sub PlotPhi()
Dim rMax As Double, rMin As Double
Dim i As Integer, j As Integer, k As Integer
Dim vPhi As Variant
If bNeedReset Then Call ResetAll
ReDim vPhi(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
For i = 1 To nNodesX: For j = 1 To nNodesY: For k = 1 To nNodesZ
    Select Case nSelectCarrier
        Case holes: vPhi(i, j, k) = vPhiP(i, j, k)
        Case electrons: vPhi(i, j, k) = vPhiN(i, j, k)
        Case combined: vPhi(i, j, k) = vPhiP(i, j, k) - vPhiN(i, j, k)
    End Select
Next k: Next j: Next i
For i = 1 To nNodesX: For j = 1 To nNodesY: For k = 1 To sTexture(i, j) - 1
    vPhi(i, j, k) = vPhi(i, j, sTexture(i, j))
Next k: Next j: Next i
currentPlot = "Quasi-Fermi"
Range(titleCell).Value = currentPlot
Call ValidatePosition(currentPlot)
Call ViewPlane(vPhi, sPlot, rMin, rMax)
Call PostData(sPlot, rMin, rMax)
Call NormalizePlot(sPlot, rMin, rMax)
Range(plotUnitsCell).Value = "V"
Range(lastPlotViewCell).Value = Range(viewCell)
Call UpdateContourChart
End Sub

Sub PlotQgamma()
Dim rMax As Double, rMin As Double
Dim i As Integer, j As Integer, k As Integer
Dim vQ As Variant
If bNeedReset Then Call ResetAll
ReDim vQ(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
For i = 1 To nNodesX: For j = 1 To nNodesY: For k = 1 To nNodesZ
    Select Case nSelectCarrier
        Case holes
            vQ(i, j, k) = Qgamma(holes, i, j, k)
        Case electrons
            vQ(i, j, k) = Qgamma(electrons, i, j, k)
        Case combined
            vQ(i, j, k) = Qgamma(holes, i, j, k) + Qgamma(electrons, i, j, k)
    End Select
Next k: Next j: Next i
currentPlot = "Qgamma"
Range(titleCell).Value = currentPlot
Call ValidatePosition(currentPlot)
Call ViewPlane(vQ, sPlot, rMin, rMax)
Call PostData(sPlot, rMin, rMax)
Call NormalizePlot(sPlot, rMin, rMax)
Range(plotUnitsCell).Value = "A/cm3"
Range(lastPlotViewCell).Value = Range(viewCell)
Call UpdateContourChart
End Sub

Sub PlotGamma()
Dim rMax As Double, rMin As Double
Dim i As Integer, j As Integer, k As Integer
Dim vGamma As Variant
If bNeedReset Then Call ResetAll
ReDim vGamma(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
Select Case nSelectCarrier
    Case holes: Call CalculateGamma(holes): vGamma = vGammaP
    Case electrons: Call CalculateGamma(electrons): vGamma = vGammaN
    Case combined: Call CalculateGamma(holes): Call CalculateGamma(electrons): vGamma = MAdd3D(vGammaP, vGammaN)
End Select
currentPlot = "Gamma"
Range(titleCell).Value = currentPlot
Call ValidatePosition(currentPlot)
Call ViewPlane(vGamma, sPlot, rMin, rMax)
Call PostData(sPlot, rMin, rMax)
Call NormalizePlot(sPlot, rMin, rMax)
Range(plotUnitsCell).Value = "A/cm"
Range(lastPlotViewCell).Value = Range(viewCell)
Call UpdateContourChart
End Sub

'*** VECTOR PLOTS ***

Sub PlotCurrent()
Dim rMax As Double, rMin As Double
Dim i As Integer, j As Integer, k As Integer
Dim localP As Double, localN As Double
Dim vJ As Variant
If bNeedReset Then Call ResetAll
ReDim vJ(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
Select Case nSelectCarrier
    Case holes: Call CalculateGamma(holes)
    Case electrons: Call CalculateGamma(electrons)
    Case combined: Call CalculateGamma(holes): Call CalculateGamma(electrons)
End Select
For i = 1 To nNodesX: For j = 1 To nNodesY: For k = sTexture(i, j) To nNodesZ
    Select Case nSelectCarrier
        Case holes
            vectorArray(xAxis, i, j, k) = CurrentDensity(True, holes, xAxis, i, j, k)
            vectorArray(yAxis, i, j, k) = CurrentDensity(True, holes, yAxis, i, j, k)
            vectorArray(zAxis, i, j, k) = CurrentDensity(True, holes, zAxis, i, j, k)
        Case electrons
            vectorArray(xAxis, i, j, k) = CurrentDensity(True, electrons, xAxis, i, j, k)
            vectorArray(yAxis, i, j, k) = CurrentDensity(True, electrons, yAxis, i, j, k)
            vectorArray(zAxis, i, j, k) = CurrentDensity(True, electrons, zAxis, i, j, k)
        Case combined
            vectorArray(xAxis, i, j, k) = CurrentDensity(True, holes, xAxis, i, j, k) + CurrentDensity(True, electrons, xAxis, i, j, k)
            vectorArray(yAxis, i, j, k) = CurrentDensity(True, holes, yAxis, i, j, k) + CurrentDensity(True, electrons, yAxis, i, j, k)
            vectorArray(zAxis, i, j, k) = CurrentDensity(True, holes, zAxis, i, j, k) + CurrentDensity(True, electrons, zAxis, i, j, k)
    End Select
    Select Case nSelectDirection
        Case xAxis: vJ(i, j, k) = vectorArray(xAxis, i, j, k)
        Case yAxis: vJ(i, j, k) = vectorArray(yAxis, i, j, k)
        Case zAxis: vJ(i, j, k) = vectorArray(zAxis, i, j, k)
        Case vector: vJ(i, j, k) = Sqr(vectorArray(xAxis, i, j, k) ^ 2 + vectorArray(yAxis, i, j, k) ^ 2 + vectorArray(zAxis, i, j, k) ^ 2)
    End Select
Next k: Next j: Next i
For i = 1 To nNodesX: For j = 1 To nNodesY: For k = 1 To sTexture(i, j) - 1
    vectorArray(xAxis, i, j, k) = vectorArray(xAxis, i, j, sTexture(i, j))
    vectorArray(yAxis, i, j, k) = vectorArray(yAxis, i, j, sTexture(i, j))
    vectorArray(zAxis, i, j, k) = vectorArray(zAxis, i, j, sTexture(i, j))
    vJ(i, j, k) = vJ(i, j, sTexture(i, j))
Next k: Next j: Next i
currentPlot = "Current"
Range(titleCell).Value = currentPlot
Call ValidatePosition(currentPlot)
Call ViewPlane(vJ, sPlot, rMin, rMax)
Call PostData(sPlot, rMin, rMax)
Call NormalizePlot(sPlot, rMin, rMax)
Range(plotUnitsCell).Value = "A/cm2"
Range(lastPlotViewCell).Value = Range(viewCell)
Call UpdateContourChart
End Sub

Sub PlotVelocity()
Dim rMax As Double, rMin As Double
Dim i As Integer, j As Integer, k As Integer
Dim localP As Double, localN As Double
Dim vJ As Variant, vVel As Variant
If bNeedReset Then Call ResetAll
ReDim vJ(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
ReDim vVel(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
Select Case nSelectCarrier
    Case holes: Call CalculateGamma(holes)
    Case electrons: Call CalculateGamma(electrons)
    Case combined: Call CalculateGamma(holes): Call CalculateGamma(electrons)
End Select
For i = 1 To nNodesX: For j = 1 To nNodesY: For k = 1 To nNodesZ
    Select Case nSelectCarrier
        Case holes
            vectorArray(xAxis, i, j, k) = CurrentDensity(True, holes, xAxis, i, j, k)
            vectorArray(yAxis, i, j, k) = CurrentDensity(True, holes, yAxis, i, j, k)
            vectorArray(zAxis, i, j, k) = CurrentDensity(True, holes, zAxis, i, j, k)
        Case electrons
            vectorArray(xAxis, i, j, k) = CurrentDensity(True, electrons, xAxis, i, j, k)
            vectorArray(yAxis, i, j, k) = CurrentDensity(True, electrons, yAxis, i, j, k)
            vectorArray(zAxis, i, j, k) = CurrentDensity(True, electrons, zAxis, i, j, k)
        Case combined
            vectorArray(xAxis, i, j, k) = CurrentDensity(True, holes, xAxis, i, j, k) + CurrentDensity(True, electrons, xAxis, i, j, k)
            vectorArray(yAxis, i, j, k) = CurrentDensity(True, holes, yAxis, i, j, k) + CurrentDensity(True, electrons, yAxis, i, j, k)
            vectorArray(zAxis, i, j, k) = CurrentDensity(True, holes, zAxis, i, j, k) + CurrentDensity(True, electrons, zAxis, i, j, k)
    End Select
    Select Case nSelectDirection
        Case xAxis: vJ(i, j, k) = vectorArray(xAxis, i, j, k)
        Case yAxis: vJ(i, j, k) = vectorArray(yAxis, i, j, k)
        Case zAxis: vJ(i, j, k) = vectorArray(zAxis, i, j, k)
        Case vector: vJ(i, j, k) = Sqr(vectorArray(xAxis, i, j, k) ^ 2 + vectorArray(yAxis, i, j, k) ^ 2 + vectorArray(zAxis, i, j, k) ^ 2)
    End Select
    Select Case nSelectCarrier
        Case holes
            localP = rNi * ExpPower((vPhiP(i, j, k) - vPsi(i, j, k) + vVp(i, j, k)) / rVt)
            vectorArray(xAxis, i, j, k) = vectorArray(xAxis, i, j, k) / (rQ * localP)
            vectorArray(yAxis, i, j, k) = vectorArray(yAxis, i, j, k) / (rQ * localP)
            vectorArray(zAxis, i, j, k) = vectorArray(zAxis, i, j, k) / (rQ * localP)
            vVel(i, j, k) = vJ(i, j, k) / (rQ * localP)
        Case electrons
            localN = rNi * ExpPower((vPsi(i, j, k) - vPhiN(i, j, k) + vVn(i, j, k)) / rVt)
            vectorArray(xAxis, i, j, k) = -vectorArray(xAxis, i, j, k) / (rQ * localN)
            vectorArray(yAxis, i, j, k) = -vectorArray(yAxis, i, j, k) / (rQ * localN)
            vectorArray(zAxis, i, j, k) = -vectorArray(zAxis, i, j, k) / (rQ * localN)
            If nSelectDirection = vector Then
                vVel(i, j, k) = vJ(i, j, k) / (rQ * localN)
            Else
                vVel(i, j, k) = -vJ(i, j, k) / (rQ * localN)
            End If
        Case combined
            localP = rNi * ExpPower((vPhiP(i, j, k) - vPsi(i, j, k) + vVp(i, j, k)) / rVt)
            localN = rNi * ExpPower((vPsi(i, j, k) - vPhiN(i, j, k) + vVn(i, j, k)) / rVt)
            vectorArray(xAxis, i, j, k) = vectorArray(xAxis, i, j, k) / (rQ * (localP + localN))
            vectorArray(yAxis, i, j, k) = vectorArray(yAxis, i, j, k) / (rQ * (localP + localN))
            vectorArray(zAxis, i, j, k) = vectorArray(zAxis, i, j, k) / (rQ * (localP + localN))
            vVel(i, j, k) = vJ(i, j, k) / (rQ * (localP + localN))
    End Select
Next k: Next j: Next i
For i = 1 To nNodesX: For j = 1 To nNodesY: For k = 1 To sTexture(i, j) - 1
    vVel(i, j, k) = vVel(i, j, sTexture(i, j))
Next k: Next j: Next i
currentPlot = "Velocity"
Range(titleCell).Value = currentPlot
Call ValidatePosition(currentPlot)
Call ViewPlane(vVel, sPlot, rMin, rMax)
Call PostData(sPlot, rMin, rMax)
Call NormalizePlot(sPlot, rMin, rMax)
Range(plotUnitsCell).Value = "cm/s"
Range(lastPlotViewCell).Value = Range(viewCell)
Call UpdateContourChart
End Sub

Sub PlotField()
Dim rMax As Double, rMin As Double
Dim i As Integer, j As Integer, k As Integer
Dim thisEx As Double, thisEy As Double, thisEz As Double
Dim vE As Variant
If bNeedReset Then Call ResetAll
ReDim vE(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
For i = 1 To nNodesX: For j = 1 To nNodesY: For k = 1 To nNodesZ
    vectorArray(xAxis, i, j, k) = ElectricField(xAxis, i, j, k)
    vectorArray(yAxis, i, j, k) = ElectricField(yAxis, i, j, k)
    vectorArray(zAxis, i, j, k) = ElectricField(zAxis, i, j, k)
    Select Case nSelectDirection
        Case xAxis: vE(i, j, k) = vectorArray(xAxis, i, j, k)
        Case yAxis: vE(i, j, k) = vectorArray(yAxis, i, j, k)
        Case zAxis: vE(i, j, k) = vectorArray(zAxis, i, j, k)
        Case vector: vE(i, j, k) = Sqr(vectorArray(xAxis, i, j, k) ^ 2 + vectorArray(yAxis, i, j, k) ^ 2 + vectorArray(zAxis, i, j, k) ^ 2)
    End Select
Next k: Next j: Next i
For i = 1 To nNodesX: For j = 1 To nNodesY: For k = 1 To sTexture(i, j) - 1
    vE(i, j, k) = vE(i, j, sTexture(i, j))
Next k: Next j: Next i
currentPlot = "Electric Field"
Range(titleCell).Value = currentPlot
Call ValidatePosition(currentPlot)
Call ViewPlane(vE, sPlot, rMin, rMax)
Call PostData(sPlot, rMin, rMax)
Call NormalizePlot(sPlot, rMin, rMax)
Range(plotUnitsCell).Value = "V/cm"
Range(lastPlotViewCell).Value = Range(viewCell)
Call UpdateContourChart
End Sub

'*** SURFACE-ONLY PLOTS ***

Sub PlotTexture()
Dim rMax As Double, rMin As Double
Dim i As Integer, j As Integer, k As Integer
Dim sRealTexture As Variant
If bNeedReset Then Call ResetAll
ReDim sRealTexture(1 To nNodesX, 1 To nNodesY) As Double
For i = 1 To nNodesX: For j = 1 To nNodesY
    sRealTexture(i, j) = deviceHeight * 10000# * (sTexture(i, j) - 1) / (nNodesZ - 1)
Next j: Next i
currentPlot = "Texture"
Range(titleCell).Value = currentPlot
Range(viewCell).Value = "Surface"
sPlot = Application.Transpose(sRealTexture)
rMin = WorksheetFunction.Min(sPlot)
rMax = WorksheetFunction.max(sPlot)
Call PostData(sPlot, rMin, rMax)
Call NormalizePlot(sPlot, rMin, rMax)
Range(plotUnitsCell).Value = "µm"
Range(lastPlotViewCell).Value = Range(viewCell)
Call UpdateContourChart
End Sub

Sub PlotQcharge()
Dim rMax As Double, rMin As Double
Dim i As Integer, j As Integer, k As Integer
If bNeedReset Then Call ResetAll
currentPlot = "Qcharge"
Range(titleCell).Value = currentPlot
Range(viewCell).Value = "Surface"
sPlot = Application.Transpose(sCharge)
rMin = WorksheetFunction.Min(sPlot)
rMax = WorksheetFunction.max(sPlot)
Call PostData(sPlot, rMin, rMax)
If -rMin > rMax Then rMax = -rMin
If rMax > -rMin Then rMin = -rMax
Call NormalizePlot(sPlot, rMin, rMax)
Range(plotUnitsCell).Value = "cm-2"
Range(lastPlotViewCell).Value = Range(viewCell)
Call UpdateContourChart
End Sub

Sub PlotSRV()
Dim rMax As Double, rMin As Double
Dim i As Integer, j As Integer, k As Integer
If bNeedReset Then Call ResetAll
currentPlot = "SRV"
Range(titleCell).Value = currentPlot
Range(viewCell).Value = "Surface"
sPlot = Application.Transpose(sVelocity)
rMin = WorksheetFunction.Min(sPlot)
rMax = WorksheetFunction.max(sPlot)
Call PostData(sPlot, rMin, rMax)
Call NormalizePlot(sPlot, rMin, rMax)
Range(plotUnitsCell).Value = "cm-2"
Range(lastPlotViewCell).Value = Range(viewCell)
Call UpdateContourChart
End Sub

Sub PlotSurfaceRecomb()
Dim rMax As Double, rMin As Double
Dim i As Integer, j As Integer, k As Integer
If bNeedReset Then Call ResetAll
currentPlot = "SurfaceRecomb"
Range(titleCell).Value = currentPlot
Range(viewCell).Value = "Surface"
sPlot = Application.Transpose(sRecombination)
rMin = WorksheetFunction.Min(sPlot)
rMax = WorksheetFunction.max(sPlot)
Call PostData(sPlot, rMin, rMax)
Call NormalizePlot(sPlot, rMin, rMax)
Range(plotUnitsCell).Value = "cm-2/s"
Range(lastPlotViewCell).Value = Range(viewCell)
Call UpdateContourChart
End Sub

'*** DRAWING SUBROUTINES ***

Sub MaskVoid()
Dim rPlotLeft As Single, rPlotTop As Single, rPlotWidth As Single, rPlotHeight As Single
Dim rLeft As Single, rTop As Single, rWidth As Single, rHeight As Single
Dim myShapes As Shapes
Dim myShape As Shape, thisShape As Shape
Dim n As Integer, nShapeCount As Integer      'Indicates number of shapes drawn to mask void
Dim strShapeNames As Variant
Dim thisShapeName As String, myShapeName As String
Dim rPosition As Double     'cm
Dim bJustAdded As Boolean
Dim lightgray As Variant
ReDim strShapeNames(1 To 5) As String
lightgray = RGB(230, 230, 230)
nShapeCount = 0
If Not (currentPlot = "Gamma" Or currentPlot = "Qgamma") And Range(viewCell) <> "Surface" Then
    rPosition = Range(positionCell) * 0.0001
    Call GetPlotLocation(rPlotLeft, rPlotTop, rPlotWidth, rPlotHeight)
    Select Case nTextureType
'*** GROOVES ***
        Case grooves
            Select Case Range(viewCell)
                Case "Face"
                    rHeight = rPlotHeight * textureDepth / deviceHeight
                    Set myShape = Sheets("Dashboard").Shapes.AddShape(msoShapeRightTriangle, rPlotLeft, rPlotTop, rPlotWidth, rHeight)
                    myShape.Flip msoFlipVertical
                    myShape.Flip msoFlipHorizontal
                Case "Side"
                    rHeight = rPlotHeight * textureDepth / deviceHeight * rPosition / deviceWidth
                    Set myShape = Sheets("Dashboard").Shapes.AddShape(msoShapeRectangle, rPlotLeft, rPlotTop, rPlotWidth, rHeight)
                Case "Top"
                    If rPosition < textureDepth Then rLeft = rPlotLeft + rPlotWidth * rPosition / textureDepth Else rLeft = rPlotLeft
                    rWidth = rPlotWidth * (1 - rPosition / textureDepth)
                    If rWidth < 0 Then rWidth = 0
                    Set myShape = Sheets("Dashboard").Shapes.AddShape(msoShapeRectangle, rLeft, rPlotTop, rWidth, rPlotHeight)
            End Select
            With myShape
                .Fill.Solid: .Fill.ForeColor.RGB = lightgray: .line.Visible = msoFalse
            End With
            nShapeCount = nShapeCount + 1: strShapeNames(nShapeCount) = myShape.Name
'*** PYRAMIDS ***
        Case pyramids       'Requires two shapes for masking void
            Select Case Range(viewCell)
                Case "Face"
                    rHeight = rPlotHeight * textureDepth / deviceHeight
                    Set myShape = Sheets("Dashboard").Shapes.AddShape(msoShapeRightTriangle, rPlotLeft, rPlotTop, rPlotWidth, rHeight)
                    With myShape
                        .Flip msoFlipVertical: .Flip msoFlipHorizontal: .Fill.Solid: .Fill.ForeColor.RGB = lightgray: .line.Visible = msoFalse
                    End With
                    nShapeCount = nShapeCount + 1: strShapeNames(nShapeCount) = myShape.Name
                    rHeight = rPlotHeight * textureDepth / deviceHeight * rPosition / deviceLength
                    Set myShape = Sheets("Dashboard").Shapes.AddShape(msoShapeRectangle, rPlotLeft, rPlotTop, rPlotWidth, rHeight)
                    With myShape
                        .Fill.Solid: .Fill.ForeColor.RGB = lightgray: .line.Visible = msoFalse
                    End With
                    nShapeCount = nShapeCount + 1: strShapeNames(nShapeCount) = myShape.Name
                Case "Side"
                    rHeight = rPlotHeight * textureDepth / deviceHeight
                    Set myShape = Sheets("Dashboard").Shapes.AddShape(msoShapeRightTriangle, rPlotLeft, rPlotTop, rPlotWidth, rHeight)
                    With myShape
                        .Flip msoFlipVertical: .Fill.Solid: .Fill.ForeColor.RGB = lightgray: .line.Visible = msoFalse
                    End With
                    nShapeCount = nShapeCount + 1: strShapeNames(nShapeCount) = myShape.Name
                    rHeight = rPlotHeight * textureDepth / deviceHeight * rPosition / deviceWidth
                    Set myShape = Sheets("Dashboard").Shapes.AddShape(msoShapeRectangle, rPlotLeft, rPlotTop, rPlotWidth, rHeight)
                    With myShape
                        .Fill.Solid: .Fill.ForeColor.RGB = lightgray: .line.Visible = msoFalse
                    End With
                    nShapeCount = nShapeCount + 1: strShapeNames(nShapeCount) = myShape.Name
                Case "Top"
                    If rPosition < textureDepth Then rLeft = rPlotLeft + rPlotWidth * rPosition / textureDepth Else rLeft = rPlotLeft + rPlotWidth
                    rWidth = rPlotLeft + rPlotWidth - rLeft
                    Set myShape = Sheets("Dashboard").Shapes.AddShape(msoShapeRectangle, rLeft, rPlotTop, rWidth, rPlotHeight)
                    With myShape
                        .Fill.Solid: .Fill.ForeColor.RGB = lightgray: .line.Visible = msoFalse
                    End With
                    nShapeCount = nShapeCount + 1: strShapeNames(nShapeCount) = myShape.Name
                    If rPosition < textureDepth Then rHeight = rPlotHeight * (1 - rPosition / textureDepth) Else rHeight = 0
                    Set myShape = Sheets("Dashboard").Shapes.AddShape(msoShapeRectangle, rPlotLeft, rPlotTop, rPlotWidth, rHeight)
                    With myShape
                        .Fill.Solid: .Fill.ForeColor.RGB = lightgray: .line.Visible = msoFalse
                    End With
                    nShapeCount = nShapeCount + 1: strShapeNames(nShapeCount) = myShape.Name
            End Select
'*** INVERTED ***
        Case inverted       'Face and Side require two shapes for masking void
            Select Case Range(viewCell)
                Case "Face"
                    rHeight = rPlotHeight * textureDepth / deviceHeight * (1 - rPosition / deviceLength)
                    rWidth = rPlotWidth * (1 - rPosition / deviceLength)
                    Set myShape = Sheets("Dashboard").Shapes.AddShape(msoShapeRightTriangle, rPlotLeft, rPlotTop, rWidth, rHeight)
                    With myShape
                        .Flip msoFlipVertical: .Flip msoFlipHorizontal: .Fill.Solid: .Fill.ForeColor.RGB = lightgray: .line.Visible = msoFalse
                    End With
                    nShapeCount = nShapeCount + 1: strShapeNames(nShapeCount) = myShape.Name
                    rHeight = rPlotHeight * textureDepth / deviceHeight * (1 - rPosition / deviceLength)
                    rWidth = rPlotWidth * rPosition / deviceLength
                    rLeft = rPlotLeft + rPlotWidth * (1 - rPosition / deviceLength)
                    Set myShape = Sheets("Dashboard").Shapes.AddShape(msoShapeRectangle, rLeft, rPlotTop, rWidth, rHeight)
                    With myShape
                        .Fill.Solid: .Fill.ForeColor.RGB = lightgray: .line.Visible = msoFalse
                    End With
                    nShapeCount = nShapeCount + 1: strShapeNames(nShapeCount) = myShape.Name
                Case "Side"
                    rHeight = rPlotHeight * textureDepth / deviceHeight * rPosition / deviceWidth
                    rWidth = rPlotWidth * rPosition / deviceLength
                    Set myShape = Sheets("Dashboard").Shapes.AddShape(msoShapeRightTriangle, rPlotLeft, rPlotTop, rWidth, rHeight)
                    With myShape
                        .Flip msoFlipVertical: .Flip msoFlipHorizontal: .Fill.Solid: .Fill.ForeColor.RGB = lightgray: .line.Visible = msoFalse
                    End With
                    nShapeCount = nShapeCount + 1: strShapeNames(nShapeCount) = myShape.Name
                    rHeight = rPlotHeight * textureDepth / deviceHeight * rPosition / deviceWidth
                    rWidth = rPlotWidth * (1 - rPosition / deviceLength)
                    rLeft = rPlotLeft + rPlotWidth * rPosition / deviceLength
                    Set myShape = Sheets("Dashboard").Shapes.AddShape(msoShapeRectangle, rLeft, rPlotTop, rWidth, rHeight)
                    With myShape
                        .Fill.Solid: .Fill.ForeColor.RGB = lightgray: .line.Visible = msoFalse
                    End With
                    nShapeCount = nShapeCount + 1: strShapeNames(nShapeCount) = myShape.Name
                Case "Top"
                    If rPosition < textureDepth Then
                        rLeft = rPlotLeft + rPlotWidth * rPosition / textureDepth
                        rTop = rPlotTop + rPlotHeight * rPosition / textureDepth
                        rWidth = rPlotWidth * (1 - rPosition / textureDepth)
                        rHeight = rPlotHeight * (1 - rPosition / textureDepth)
                        Set myShape = Sheets("Dashboard").Shapes.AddShape(msoShapeRectangle, rLeft, rTop, rWidth, rHeight)
                        With myShape
                            .Fill.Solid: .Fill.ForeColor.RGB = lightgray: .line.Visible = msoFalse
                        End With
                        nShapeCount = nShapeCount + 1: strShapeNames(nShapeCount) = myShape.Name
                End If
            End Select
    End Select
End If
'Remove all void shapes from Dashboard except the ones just added
'This is done after drawing instead of before to reduce screen "flicker"
Set myShapes = Sheets("Dashboard").Shapes
For Each thisShape In myShapes
    thisShapeName = thisShape.Name
    bJustAdded = False
    For n = 1 To nShapeCount
        If thisShapeName = strShapeNames(n) Then bJustAdded = True
    Next n
    If Not bJustAdded And (thisShape.AutoShapeType = msoShapeRightTriangle Or thisShape.AutoShapeType = msoShapeRectangle) Then thisShape.Delete
Next thisShape
End Sub

Sub DisplayArrows(ByVal aspect As Single)
'Displays directional arrows for the current view of vArray based on the values in vArray (x, y, z volumetric data)
'First removes all connector shapes used to draw arrows
'Arrows are displayed 10 in each axis, 100 total
'To avoid confusing display coordinates with solution-volume coordinates, the display coordinates are u (abscissa), v (ordinate), and w (out-of-plane)
'Calls Arrow3D to actually draw the arrow on the display
Dim i As Integer, j As Integer, k As Integer, nPosition As Integer
Dim rMagnitude As Double, rMaxMagnitude As Double
Dim rPositionU As Double, rPositionV As Double, rLengthU As Double, rLengthV As Double, rLengthW As Double
Dim myShapes As Shapes, thisShape As Shape
Dim vArrayX As Variant, vArrayY As Variant, vArrayZ As Variant
Dim pArrayX As Variant, pArrayY As Variant, pArrayZ As Variant
Dim pArrayU As Variant, pArrayV As Variant, pArrayW As Variant
Dim pArrayS As Variant          'Location of front texture as a fraction of deviceHeight
ReDim vArrayX(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
ReDim vArrayY(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
ReDim vArrayZ(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
ReDim pArrayX(1 To nPlotNodes, 1 To nPlotNodes, 1 To nPlotNodes) As Double
ReDim pArrayY(1 To nPlotNodes, 1 To nPlotNodes, 1 To nPlotNodes) As Double
ReDim pArrayZ(1 To nPlotNodes, 1 To nPlotNodes, 1 To nPlotNodes) As Double
ReDim pArrayU(1 To nPlotNodes, 1 To nPlotNodes) As Double
ReDim pArrayV(1 To nPlotNodes, 1 To nPlotNodes) As Double
ReDim pArrayW(1 To nPlotNodes, 1 To nPlotNodes) As Double
ReDim pArrayX(1 To nPlotNodes, 1 To nPlotNodes) As Double
'Delete all connector shapes from the Dashboard
Set myShapes = Sheets("Dashboard").Shapes
For Each thisShape In myShapes
    If thisShape.Connector Or thisShape.AutoShapeType = msoShapeOval Then thisShape.Delete
Next thisShape
'Exit if the current plot is not directional or if selected direction is not vector
If Not DirectionalPlot Or nSelectDirection <> vector Then Exit Sub
'Assign vector array values to three scalar arrays, find maximum magnitude as well
'Assign zero in the etched void and at the surface
rMaxMagnitude = 0
For i = 1 To nNodesX: For j = 1 To nNodesY: For k = 1 To nNodesZ
    rMagnitude = vectorArray(xAxis, i, j, k) ^ 2 + vectorArray(yAxis, i, j, k) ^ 2 + vectorArray(zAxis, i, j, k) ^ 2
    If rMagnitude > rMaxMagnitude Then rMaxMagnitude = rMagnitude
    vArrayX(i, j, k) = vectorArray(xAxis, i, j, k)
    vArrayY(i, j, k) = vectorArray(yAxis, i, j, k)
    vArrayZ(i, j, k) = vectorArray(zAxis, i, j, k)
Next k: Next j: Next i
rMaxMagnitude = Sqr(rMaxMagnitude)
If rMaxMagnitude = 0 Then rMaxMagnitude = 1     'Avoid irrelevant division by zero
'Adjust resolution to nPlotNodes-1 in each axis
pArrayX = MTrans3Dback(MMult3D(r2tZ, MTrans3Drp(MMult3D(r2tY, MTrans3Drc(MMult3D(r2tX, vArrayX))))))
pArrayY = MTrans3Dback(MMult3D(r2tZ, MTrans3Drp(MMult3D(r2tY, MTrans3Drc(MMult3D(r2tX, vArrayY))))))
pArrayZ = MTrans3Dback(MMult3D(r2tZ, MTrans3Drp(MMult3D(r2tY, MTrans3Drc(MMult3D(r2tX, vArrayZ))))))
pArrayS = Application.Transpose(Application.MMult(r2tY, Application.Transpose(Application.MMult(r2tX, sTexture))))
For i = 1 To nPlotNodes: For j = 1 To nPlotNodes
    pArrayS(i, j) = (pArrayS(i, j) - 1) / (nNodesZ - 1) * (nPlotNodes - 1) + 1
Next j: Next i
'Translate data for the given view perspective and set out-of-plane postion
Select Case Range(viewCell)
    Case "Face"
        nPosition = nPlotNodes - Round((nPlotNodes - 1) * Range(positionCell) / (10000# * deviceLength), 0)
        For i = 1 To nPlotNodes: For j = 1 To nPlotNodes
            If j > pArrayS(i, nPosition) Then       'Only display arrows not in the void
                pArrayU(i, j) = pArrayX(i, nPosition, j) * deviceHeight / deviceWidth     'Correct for possiible distortion of square contour plot
                pArrayV(i, j) = pArrayZ(i, nPosition, j)
                pArrayW(i, j) = pArrayY(i, nPosition, j)
            End If
        Next j: Next i
    Case "Side"
        nPosition = Round((nPlotNodes - 1) * Range(positionCell) / (10000# * deviceWidth), 0) + 1
        For i = 1 To nPlotNodes: For j = 1 To nPlotNodes
            If j > pArrayS(nPosition, i) Then       'Only display arrows not in the void
                pArrayU(i, j) = pArrayY(nPosition, i, j) * deviceHeight / deviceLength      'Correct for possiible distortion of square contour plot
                pArrayV(i, j) = pArrayZ(nPosition, i, j)
                pArrayW(i, j) = -pArrayX(nPosition, i, j)
        End If
        Next j: Next i
    Case "Top"
        nPosition = Round((nPlotNodes - 1) * Range(positionCell) / (10000# * deviceHeight), 0) + 1
        For i = 1 To nPlotNodes: For j = 1 To nPlotNodes
            If nPosition > pArrayS(i, j) Then       'Only display arrows not in the void
                pArrayU(i, j) = pArrayX(i, j, nPosition) * deviceLength / deviceWidth   'Correct for possible distortion of square contour plot
                pArrayV(i, j) = pArrayY(i, j, nPosition)
                pArrayW(i, j) = -pArrayZ(i, j, nPosition)
            End If
        Next j: Next i
    Case "Surface"
        For i = 1 To nPlotNodes: For j = 1 To nPlotNodes
            pArrayU(i, j) = pArrayX(i, j, pArrayS(i, j)) * deviceLength / deviceWidth  'Correct for possible distortion of square contour plot
            pArrayV(i, j) = pArrayY(i, j, pArrayS(i, j))
            pArrayW(i, j) = -pArrayZ(i, j, pArrayS(i, j))
        Next j: Next i
End Select
'Draw arrows in a sparse grid. Scale length to the square root of the vector magnitude to better reveal minor flows
For i = 1 To nPlotNodes: For j = 1 To nPlotNodes
    If i Mod 4 = 3 And j Mod 4 = 3 Then
        rPositionU = (i - 1) / (nPlotNodes - 1)
        rPositionV = (j - 1) / (nPlotNodes - 1)
        rLengthU = pArrayU(i, j) / rMaxMagnitude
        rLengthV = pArrayV(i, j) / rMaxMagnitude
        rLengthW = pArrayW(i, j) / rMaxMagnitude
        rMagnitude = Sqr(rLengthU ^ 2 + rLengthV ^ 2 + rLengthW ^ 2)
        If rMagnitude = 0 Then rMagnitude = 1   'Avoid irrelevant division by zero
        rLengthU = 0.1 * rLengthU / Sqr(rMagnitude)
        rLengthV = 0.1 * rLengthV / Sqr(rMagnitude)
        rLengthW = 0.1 * rLengthW / Sqr(rMagnitude)
        Call Arrow3D(rPositionU, rPositionV, rLengthU, rLengthV, rLengthW, aspect)
    End If
Next j: Next i
End Sub

Sub Arrow3D(ByVal locationU As Double, ByVal locationV As Double, ByVal lengthU As Double, ByVal lengthV As Double, ByVal lengthW As Double, ByVal aspect As Single)
'The location coordinates are right and down from the upper-left corner, normalized to the width and height of the contour plot
'The length coordinates are positive to the right (U), down (V), and out of the screen (W), normalized to the contour plot width
'The height-to-width aspect ratio can be adjusted to prepare for scaled image generation
'If aspect=0 then the arrow is not drawn
Dim rLeft As Single, rTop As Single, rWidth As Single, rHeight As Single
Dim rCenterU As Single, rCenterV As Single, rLengthU As Single, rLengthV As Single, rLengthW As Single
Dim myShape As Shape
If aspect = 0 Then Exit Sub
Call GetPlotLocation(rLeft, rTop, rWidth, rHeight)  'Establish the display coordinates for the plot area
rCenterU = rLeft + locationU * rWidth
rCenterV = rTop + locationV * rHeight
rLengthU = lengthU * rWidth
rLengthV = lengthV * rHeight
rLengthW = lengthW * Sqr((rWidth ^ 2 + rHeight ^ 2) / 2)
'Plot the arrow that's within the view plane, if long enough to justify doing so
If Sqr(rLengthU ^ 2 + rLengthV ^ 2) > 5 Then
    Set myShape = Sheets("Dashboard").Shapes.AddConnector(msoConnectorStraight, rCenterU, rCenterV, rCenterU + rLengthU, rCenterV + rLengthV)
    myShape.line.EndArrowheadStyle = msoArrowheadTriangle
    myShape.line.EndArrowheadLength = msoArrowheadShort
    myShape.line.EndArrowheadWidth = msoArrowheadNarrow
    myShape.line.ForeColor.RGB = RGB(255, 255, 255)
    myShape.line.weight = 1
End If
'Plot the arrow that's normal to the view plane, if long enough to justify doing so
If lengthW > 0 And Abs(rLengthW) > 3 Then
    Set myShape = Sheets("Dashboard").Shapes.AddShape(msoShapeOval, rCenterU - aspect * rLengthW / 2, rCenterV - rLengthW / 2, aspect * rLengthW, rLengthW)
    myShape.line.ForeColor.RGB = RGB(255, 255, 255)
    myShape.Fill.Visible = msoFalse
    myShape.line.weight = 1
    Set myShape = Sheets("Dashboard").Shapes.AddShape(msoShapeOval, rCenterU - aspect * rLengthW / 10, rCenterV - rLengthW / 10, aspect * rLengthW / 5, rLengthW / 5)
    myShape.line.ForeColor.RGB = RGB(255, 255, 255)
    myShape.Fill.ForeColor.RGB = RGB(255, 255, 255)
    myShape.Fill.Solid
End If
If lengthW < 0 And Abs(rLengthW) > 3 Then
    Set myShape = Sheets("Dashboard").Shapes.AddConnector(msoConnectorStraight, rCenterU - aspect * rLengthW / 2, rCenterV - rLengthW / 2, rCenterU + aspect * rLengthW / 2, rCenterV + rLengthW / 2)
    myShape.line.ForeColor.RGB = RGB(255, 255, 255)
    myShape.line.weight = 1
    Set myShape = Sheets("Dashboard").Shapes.AddConnector(msoConnectorStraight, rCenterU - aspect * rLengthW / 2, rCenterV + rLengthW / 2, rCenterU + aspect * rLengthW / 2, rCenterV - rLengthW / 2)
    myShape.line.ForeColor.RGB = RGB(255, 255, 255)
    myShape.line.weight = 1
End If
End Sub

Sub CreateImage()
Const initialOffset = 30                    'Offset the image right and down from the currently selected cell
Const offsetLeft = -12, offsetTop = -12     'These offsets are introduced by Excel copy/paste
Dim plotWidth As Double, plotHeight As Double
Dim chartWidth As Variant, chartHeight As Variant
Dim aspectPicture As Double, aspectDevice As Double
Dim myShapes As Shapes, thisShape As Shape
Dim nCount As Integer
Dim shapeNameArray(1 To 500) As String

bImage = True       'Prevents reset-all when accessing contour plot on Mac
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

 'Adjust aspect ratio to prepare for image resizing
Select Case Range(viewCell)
    Case "Face": Call DisplayArrows(deviceHeight / deviceWidth)
    Case "Side": Call DisplayArrows(deviceHeight / deviceLength)
    Case "Top": Call DisplayArrows(deviceLength / deviceWidth)
    Case "Surface": Call DisplayArrows(deviceLength / deviceWidth)
End Select
Sheets("Dashboard").ChartObjects(contourPlot).Activate
nCount = 1
shapeNameArray(nCount) = contourPlot
Set myShapes = Sheets("Dashboard").Shapes
For Each thisShape In myShapes
    If thisShape.AutoShapeType = msoShapeIsoscelesTriangle Or _
                 thisShape.AutoShapeType = msoShapeRightTriangle Or _
                 thisShape.AutoShapeType = msoShapeRectangle Or _
                 thisShape.AutoShapeType = msoShapeOval Or _
                 thisShape.Connector Then
        nCount = nCount + 1
        shapeNameArray(nCount) = thisShape.Name
        thisShape.Select (True)
    End If
Next thisShape
Sheets("Dashboard").Shapes.Range(shapeNameArray).Select
Selection.Copy
Sheets("Dashboard").Range(programNameCell).Select

Sheets("Image").Select
ActiveWindow.Zoom = 100
ActiveSheet.Pictures.Paste.Select
Selection.ShapeRange.IncrementLeft initialOffset
Selection.ShapeRange.IncrementTop initialOffset
Selection.ShapeRange.PictureFormat.cropLeft = (chartWidth - plotWidth) / 2
Selection.ShapeRange.PictureFormat.cropRight = (chartWidth - plotWidth) / 2
Selection.ShapeRange.PictureFormat.cropTop = (chartHeight - plotHeight) / 2
Selection.ShapeRange.PictureFormat.cropBottom = (chartHeight - plotHeight) / 2
Selection.ShapeRange.LockAspectRatio = msoFalse
Selection.Copy
    
If nSolutionMode = lateral Then       'Symmetry does not allow unfolding
    'Adjust aspect ratio of image to match actual device
     If Range(viewCell) = "Top" Or Range(viewCell) = "Surface" Then
         aspectPicture = Selection.ShapeRange.Height / Selection.ShapeRange.Width
         aspectDevice = deviceLength / deviceWidth
         Selection.ShapeRange.ScaleWidth aspectPicture / aspectDevice, msoFalse
     End If
     If Range(viewCell) = "Face" Then
         aspectPicture = Selection.ShapeRange.Height / Selection.ShapeRange.Width
         aspectDevice = deviceHeight / deviceWidth
         Selection.ShapeRange.ScaleWidth aspectPicture / aspectDevice, msoFalse
     End If
     If Range(viewCell) = "Side" Then
         aspectPicture = Selection.ShapeRange.Height / Selection.ShapeRange.Width
         aspectDevice = deviceHeight / deviceLength
         Selection.ShapeRange.ScaleWidth aspectPicture / aspectDevice, msoFalse
     End If
Else            'Symmetry allows unfolding
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
         Selection.ShapeRange.ScaleWidth aspectPicture / aspectDevice, msoFalse
     End If
     If Range(viewCell) = "Face" Then
         aspectPicture = 4 * Selection.ShapeRange.Height / Selection.ShapeRange.Width
         aspectDevice = deviceHeight / deviceWidth
         Selection.ShapeRange.ScaleWidth aspectPicture / aspectDevice, msoFalse
     End If
     If Range(viewCell) = "Side" Then
         aspectPicture = 4 * Selection.ShapeRange.Height / Selection.ShapeRange.Width
         aspectDevice = deviceHeight / deviceLength
         Selection.ShapeRange.ScaleWidth aspectPicture / aspectDevice, msoFalse
     End If
End If
Call DisplayArrows(1)       'Restore unity aspect ratio
bImage = False
End Sub


