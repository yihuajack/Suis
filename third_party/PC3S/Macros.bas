Attribute VB_Name = "Macros"
Option Explicit

Sub MacroRecombination()
'Calculate recombination at three voltages: low, medium, and high
'From the results, extract values of Gsh, Jo2, and Jo1
Const V1 As Double = 0.3, V2 As Double = 0.5, V3 As Double = 0.7
Dim J1 As Double, J2 As Double, J3 As Double
Dim rGsh As Double, rJo2 As Double, rJo1 As Double, RMSerror As Double
Dim ivData As Variant
If Not bDebug Then On Error GoTo ErrHandler:
bScan = True
bEnableChangeHandlers = False
Call ResetOutputs
Range(macroGshCell).Value = ""
Range(macroJo2Cell).Value = ""
Range(macroJo1Cell).Value = ""
Range(illuminationCell).Value = "Dark"
If Range(contactModeCell) <> "Floating" And Range(contactModeCell) <> "FlatBand" Then Range(contactModeCell).Value = "Bipolar"
bEnableChangeHandlers = True
rStartTime = Timer
Call ResetAll
'Low voltage
currentPlot = "Potential"
Range(voltageCell).Value = V1
Call MapInputParameters
Call SolveNodes
J1 = Range(totalRecombinationCell)
'High voltage
currentPlot = "Potential"
Range(voltageCell).Value = V3
Call MapInputParameters
Call SolveNodes
J3 = Range(totalRecombinationCell)
'Medium voltage
currentPlot = "Potential"
Range(voltageCell).Value = V2
Call MapInputParameters
Call SolveNodes
J2 = Range(totalRecombinationCell)
'Calculate recombination parameters including shunt
ReDim ivData(1 To 3, 1 To 2) As Double
ivData(1, 1) = V1: ivData(1, 2) = J1
ivData(2, 1) = V2: ivData(2, 2) = J2
ivData(3, 1) = V3: ivData(3, 2) = J3
If Not TwoDiodeFit(ivData, True, True, True, rJo1, rJo2, rGsh) Then
    If rGsh < 0 Then Call TwoDiodeFit(ivData, False, True, True, rJo1, rJo2, rGsh)
    If rJo1 < 0 Then Call TwoDiodeFit(ivData, True, True, False, rJo1, rJo2, rGsh)
    If rJo2 < 0 Then Call TwoDiodeFit(ivData, True, False, True, rJo1, rJo2, rGsh)
End If
Range(macroGshCell).Value = 1000# * rGsh
Range(macroJo2Cell).Value = 1000000000# * rJo2
Range(macroJo1Cell).Value = 1E+15 * rJo1
Range(dataIVCells) = ivData
Range(progressCell).Value = "Done"
bScan = False
Exit Sub

ErrHandler:
    Range(progressCell) = "Error"
    bScan = False
End Sub

Sub MacroSheetRho()
'Calculate sheet resistance by applying a small lateral voltage
If Not bDebug Then On Error GoTo ErrHandler:
If nSurfaceType = nBackgroundType Then
    MsgBox ("Sheet Rho requires a dissimilar-type diffused layer")
    Exit Sub
End If
If bNeedReset Then Call ResetAll
bScan = True
bEnableChangeHandlers = False
Call ResetOutputs
Range(macroSheetRhoCell).Value = ""
If Range(voltageCell) = 0 Then Range(voltageCell).Value = 0.5
Range(illuminationCell).Value = "Dark"
Range(contactModeCell).Value = "Lateral"
bEnableChangeHandlers = True
rStartTime = Timer
Call MapInputParameters
Call SolveNodes
If Range(lateralCurrentCell) > 0 Then Range(macroSheetRhoCell).Value = 0.001 * Range(voltageCell) / Range(lateralCurrentCell)
Range(sheetRhoVoltageCell).Value = Range(voltageCell)
Range(sheetRhoCurrentCell).Value = Range(lateralCurrentCell)
Range(progressCell).Value = "Done"
bScan = False
Exit Sub

ErrHandler:
    Range(progressCell) = "Error"
    bScan = False
End Sub

Sub MacroSpectral()
If Not bDebug Then On Error GoTo ErrHandler:
If bNeedReset Then Call ResetAll
bScan = True
bEnableChangeHandlers = False
Call ResetElectrical
Call ResetGeneration
Call ResetOutputs
Call ClearSpectralResults
rStartTime = Timer
Call MapInputParameters
Call MacroFastSpectral
Range(progressCell).Value = "Done"
bScan = False
Exit Sub
    
ErrHandler:
    bScan = False
    Range(progressCell) = "Error"
End Sub

Sub MacroFastSpectral()
'Find transmission fraction by convolving collection efficiency (from dark excess pn product) with generation profile.
'This does not account for nonlinear recombination effects (n=2, high-level injection, etc.).
Dim i As Integer, j As Integer, k As Integer, n As Integer
Const Vdark As Double = 0.5
Dim lambda As Double
Dim saveIllumination As Boolean, saveSpectrum As Boolean, saveMono As Boolean
Dim reflectedFraction As Double, absorbedFraction As Double, collectedFraction As Double, maxCollEff As Double
Dim vCollEff As Variant, vCollection As Variant, spectralData As Variant
ReDim vCollEff(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
ReDim vCollection(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
Const smallPower As Double = 0.001       'Illumination power used to calculate generation profile

'Forward bias in the dark to find collection efficiency.
'Bias voltage selected to roughly approximate peak carrier densities under one-sun short-circuit conditions in a typical solar cell.
saveIllumination = bIllumination: saveSpectrum = bSpectrum: saveMono = bMono
currentPlot = "Quasi-Fermi"
nSelectCarrier = combined
Range(selectCarrierCell).Value = "Combined"
nSolutionMode = bipolar
Range(contactModeCell).Value = "Bipolar"
rVoltage = Vdark: Range(voltageCell).Value = rVoltage
bIllumination = False: Range(illuminationCell).Value = "Dark"
Range(progressCell).Value = "Dark Bias": DoEvents
Call InitializePhi
If Not SolveNodes Then Exit Sub
maxCollEff = 0
For i = 1 To nNodesX: For j = 1 To nNodesY: For k = sTexture(i, j) To nNodesZ
    vCollEff(i, j, k) = ExpPower((vPhiP(i, j, k) - vPhiN(i, j, k)) / rVt) - 1
    If vCollEff(i, j, k) > maxCollEff Then maxCollEff = vCollEff(i, j, k)
Next k: Next j: Next i
For i = 1 To nNodesX: For j = 1 To nNodesY: For k = sTexture(i, j) To nNodesZ
    If maxCollEff > 0 Then vCollEff(i, j, k) = vCollEff(i, j, k) / maxCollEff
Next k: Next j: Next i

'Now apply monochromatic illumination of desired spatial distribution at each desired wavelength
bSpectrum = False
bMono = True
bIllumination = True
spectralData = Range(spectralRange)
Range(progressCell).Value = "Spectral"
For n = LBound(spectralData, 1) To UBound(spectralData, 1)
    lambda = spectralData(n, 1)
    Call CalculatePhotogeneration(lambda, smallPower, reflectedFraction, absorbedFraction, vGeneration)
    For i = 1 To nNodesX: For j = 1 To nNodesY: For k = sTexture(i, j) To nNodesZ
        vCollection(i, j, k) = vGeneration(i, j, k) * vCollEff(i, j, k)
    Next k: Next j: Next i
    collectedFraction = VolumeAverage(vCollection) / VolumeAverage(vGeneration)
    Range(dataSpectralCells).Cells(n, 1).Value = lambda
    Range(dataSpectralCells).Cells(n, 2).Value = reflectedFraction
    Range(dataSpectralCells).Cells(n, 3).Value = absorbedFraction
    Range(dataSpectralCells).Cells(n, 4).Value = collectedFraction
    Range(spectralRange).Cells(n, 2).Value = (1 - reflectedFraction - absorbedFraction) * collectedFraction * 100
Next n
bIllumination = saveIllumination: bSpectrum = saveSpectrum: bMono = saveMono
Call ResetGeneration
Range(solutionTimeCell).Value = Timer - rStartTime
End Sub

Sub ClearMacroResults()
Range(macroSheetRhoCell).Value = ""
Range(macroJo1Cell).Value = ""
Range(macroJo2Cell).Value = ""
Range(macroGshCell).Value = ""
Range(dataIVCells).ClearContents
Range(sheetRhoVoltageCell).Value = ""
Range(sheetRhoCurrentCell).Value = ""
ClearSpectralResults
End Sub

Sub ClearSpectralResults()
Range(dataSpectralCells).ClearContents
Range(transmissionRange).ClearContents
End Sub

