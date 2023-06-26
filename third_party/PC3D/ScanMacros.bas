Attribute VB_Name = "ScanMacros"
Option Explicit

Sub MacroIV()
If Not bDebug Then On Error GoTo ErrHandler:
If bNeedReset Then Call ResetAll
bScan = True
Call ResetGeneration        'Resets of generation and electrical are done here, not in the individual macros,
Call ResetElectrical        'so that the individual ones can be called in Batch without resetting anything.
Call InitializeGammaPsi
Call ClearIV
Call RefreshCharts
If Range(ivTypeCell) = "Light IV" Then Call MacroLightIV
If Range(ivTypeCell) = "Dark IV" Then Call MacroDarkIV
If Range(ivTypeCell) = "Pseudo IV" Then Call MacroPseudoIV
If Range(ivTypeCell) = "SunsVoc" Then Call MacroSunsVoc
Range(progressCell).Value = "Scan Done"
bScan = False
Call UpdateCircuitModel
Exit Sub
    
ErrHandler:
    Range(progressCell) = "Error"
    bScan = False
End Sub

Sub MacroLightIV()
Dim i As Integer, nIncrements As Integer, n As Integer
Dim c As Double
Dim Voc As Double, Isc As Double, rJL As Double, rJo1 As Double, rJo2 As Double, rGsh As Double, rRs As Double, rError As Double
Dim Vmax As Double, Imax As Double, Pmax As Double
Dim frontSuns As Double, rearSuns As Double
Dim Pspectrum As Double, Pfront As Double, Prear As Double
Dim dataIV As Variant
dataIV = Range(IVrange)
If currentPlot = "Layout" Or currentPlot = "Generation" Then currentPlot = "Excess Carriers"
Call SolveGeneration
Pfront = 0: Prear = 0
If bSpectrum Then
    Pspectrum = 0
    For n = 1 To UBound(rSpectrumData, 1)
        Pspectrum = Pspectrum + rSpectrumData(n, 2) / 10000
    Next n
    Pfront = Pfront + Pspectrum * rFrontSuns * SurfaceAverage(1, nNodesX, 1, nNodesY, sFrontIllumination)
    Prear = Prear + Pspectrum * rRearSuns * SurfaceAverage(1, nNodesX, 1, nNodesY, sRearIllumination)
    Range(illuminationPowerCell).Value = Pfront + Prear
End If
If bMono Then
    Pfront = Pfront + rFrontMonoPower * SurfaceAverage(1, nNodesX, 1, nNodesY, sFrontIllumination)
    Prear = Prear + rRearMonoPower * SurfaceAverage(1, nNodesX, 1, nNodesY, sRearIllumination)
    Range(illuminationPowerCell).Value = Pfront + Prear
End If
If Pfront + Prear = 0 Then
    If Not bBatch Then MsgBox ("No illumination!")
    Range(progressCell).Value = "Null"
    Exit Sub
End If
If Pfront <> Pspectrum Or Prear <> 0 Or Not bSpectrum Then
    Range(ivWarningCell).Value = "Non-standard illumination"
Else
    If Pfront > 0 And bFilter Then Range(ivWarningCell).Value = "Filtered illumination"
End If

Voc = VocEstimate(rTotalGeneration)
bOpenCircuit = True: Range(openCircuitCell).Value = "Voc"
rVoltage = Voc: Range(voltageCell).Value = rVoltage
Call InitializeGammaPsi
Range(progressCell).Value = "Voc": DoEvents
If Not SolveSurfaces Then Exit Sub
Voc = ContactAverage(holes, 1, nNodesX, 1, nNodesY, sPhi) - ContactAverage(electrons, 1, nNodesX, 1, nNodesY, sPhi)

nIncrements = Range(voltageIncrementsCell)
If nIncrements > 1 Then c = (1 - 3 / nIncrements) / Log(nIncrements) Else c = 0
dataIV(2, 1) = 0
For i = 1 To nIncrements
    dataIV(i + 2, 1) = Voc * (1 + c * Log(i / nIncrements))
Next i
Range(IVrange) = dataIV

bOpenCircuit = False: Range(openCircuitCell).Value = "Contact"
rVoltage = 0: Range(voltageCell).Value = rVoltage
Range(progressCell).Value = "Jsc": DoEvents
Call InitializeGammaPsi
If SolveSurfaces Then
    dataIV(2, 1) = Range(terminalVoltageCell)
    dataIV(2, 2) = Range(terminalCurrentCell)
    dataIV(2, 3) = Range(terminalPowerCell)
End If
Range(IVrange) = dataIV
Call RefreshCharts

For i = 1 To nIncrements
    rVoltage = Voc * (1 + c * Log(i / nIncrements)): Range(voltageCell).Value = rVoltage
    Range(progressCell).Value = "Volts-step": DoEvents
    Call InitializeGammaPsi
    If SolveSurfaces Then  'Skip individual points that don't converge
        dataIV(i + 2, 1) = Range(terminalVoltageCell)
        dataIV(i + 2, 2) = Range(terminalCurrentCell)
        dataIV(i + 2, 3) = Range(terminalPowerCell)
    End If
    Range(IVrange) = dataIV
    Call RefreshCharts
Next i
If FindVoc(dataIV, Voc) Then Range(vocCell).Value = Voc
If FindIsc(dataIV, Isc) Then Range(iscCell).Value = Isc
If FindPmax(dataIV, Vmax, Imax, Pmax) Then
    Range(pmaxCell) = Pmax
    If Voc > 0 And Isc > 0 Then Range(ffCell).Value = Pmax / (Voc * Isc)
    rVoltage = Vmax + Imax * rSeries: Range(voltageCell).Value = rVoltage
    Range(progressCell).Value = "Pmax": DoEvents
    Call InitializeGammaPsi
    If SolveSurfaces Then
        dataIV(1, 1) = Range(terminalVoltageCell)
        dataIV(1, 2) = Range(terminalCurrentCell)
        dataIV(1, 3) = Range(terminalPowerCell)
    End If
    Pmax = Range(terminalPowerCell)
    Range(pmaxCell).Value = Pmax
    Range(IVrange) = dataIV
    If Voc > 0 And Isc > 0 Then Range(ffCell).Value = Pmax / (Voc * Isc)
    If Pfront + Prear > 0 Then Range(efficiencyCell).Value = 100 * Pmax / (Pfront + Prear)
    Call RefreshCharts
Else
    If Not bBatch Then MsgBox ("Max-power point not found")
End If
End Sub

Sub MacroDarkIV()
Dim i As Integer, j As Integer, nIncrements As Integer
Dim maxVoltage As Double, c As Double
Dim dataIV As Variant
dataIV = Range(IVrange)
nIncrements = Range(voltageIncrementsCell)
If nIncrements > 1 Then c = (1 - 2 / nIncrements) / Log(nIncrements) Else c = 0
maxVoltage = Range(darkIVmaxCell)
dataIV(nIncrements + 2, 1) = maxVoltage
ReDim vGeneration(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double   'Make it dark
Range(generationCell).Value = 0
If currentPlot = "Layout" Or currentPlot = "Generation" Then currentPlot = "Excess Carriers"
bOpenCircuit = False: Range(openCircuitCell).Value = "Contact"
For i = 1 To nIncrements
    rVoltage = maxVoltage * (1 + c * Log(i / nIncrements))
    Range(voltageCell).Value = rVoltage: Range(progressCell).Value = "Volts-step": DoEvents
    Call InitializeGammaPsi
    If SolveSurfaces Then
        dataIV(i + 2, 1) = Range(terminalVoltageCell)
        dataIV(i + 2, 2) = Abs(Range(terminalCurrentCell))
    End If
    Range(IVrange) = dataIV
    Call RefreshCharts
Next i
End Sub

Sub MacroPseudoIV()
Dim i As Integer, nIncrements As Integer, n As Integer
Dim rSuns As Double, rJo1 As Double, rJsc As Double, Voc As Double, Isc As Double
Dim Vmax As Double, Imax As Double, Pmax As Double
Dim Pspectrum As Double, Pfront As Double, Prear As Double
Dim saveTotalGen As Double
Dim saveGen As Variant, dataIV As Variant
ReDim saveGen(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double

dataIV = Range(IVrange)
If currentPlot = "Layout" Or currentPlot = "Generation" Then currentPlot = "Excess Carriers"
Call SolveGeneration
saveGen = vGeneration
saveTotalGen = rTotalGeneration
Pfront = 0: Prear = 0
If bSpectrum Then
    Pspectrum = 0
    For n = 1 To UBound(rSpectrumData, 1)
        Pspectrum = Pspectrum + rSpectrumData(n, 2) / 10000
    Next n
    Pfront = Pfront + Pspectrum * rFrontSuns * SurfaceAverage(1, nNodesX, 1, nNodesY, sFrontIllumination)
    Prear = Prear + Pspectrum * rRearSuns * SurfaceAverage(1, nNodesX, 1, nNodesY, sRearIllumination)
    Range(illuminationPowerCell).Value = Pfront + Prear
End If
If bMono Then
    Pfront = Pfront + rFrontMonoPower * SurfaceAverage(1, nNodesX, 1, nNodesY, sFrontIllumination)
    Prear = Prear + rRearMonoPower * SurfaceAverage(1, nNodesX, 1, nNodesY, sRearIllumination)
    Range(illuminationPowerCell).Value = Pfront + Prear
End If
If Pfront + Prear = 0 Then
    If Not bBatch Then MsgBox ("No illumination!")
    Range(progressCell).Value = "Null"
    Exit Sub
End If
If Pfront <> Pspectrum Or Prear <> 0 Or Not bSpectrum Then
    Range(ivWarningCell).Value = "Non-standard illumination"
Else
    If Pfront > 0 And bFilter Then Range(ivWarningCell).Value = "Filtered illumination"
End If

rJsc = q * rTotalGeneration
Voc = VocEstimate(rTotalGeneration)
rJo1 = rJsc / (Exp(Voc / rVt) - 1)
nIncrements = Range(voltageIncrementsCell)
dataIV(2, 1) = 0
dataIV(2, 2) = q * rTotalGeneration
dataIV(2, 3) = 0
dataIV(nIncrements + 2, 1) = Voc
Range(IVrange) = dataIV
Call RefreshCharts
bOpenCircuit = True: Range(openCircuitCell).Value = "Voc"

For i = 1 To nIncrements
    rSuns = Exp(-5 * Sqr(nIncrements) * ((nIncrements - i) / nIncrements) ^ 2)
    vGeneration = MScale3D(saveGen, rSuns)
    rTotalGeneration = rSuns * saveTotalGen
    Voc = rVt * Log(1 + rJsc * rSuns / rJo1)
    rVoltage = Voc: Range(voltageCell).Value = rVoltage
    Range(progressCell).Value = "Suns-step": DoEvents
    Call InitializeGammaPsi
    If SolveSurfaces Then
        Voc = ContactAverage(holes, 1, nNodesX, 1, nNodesY, sPhi) - ContactAverage(electrons, 1, nNodesX, 1, nNodesY, sPhi)
        dataIV(i + 2, 1) = Voc
        dataIV(i + 2, 2) = rJsc - rJsc * rSuns
        dataIV(i + 2, 3) = Voc * (rJsc - rJsc * rSuns)
    End If
    Range(IVrange) = dataIV
    Call RefreshCharts
Next i
If FindVoc(dataIV, Voc) Then Range(vocCell).Value = Voc
If FindIsc(dataIV, Isc) Then Range(iscCell).Value = Isc
If FindPmax(dataIV, Vmax, Imax, Pmax) Then
    vGeneration = MScale3D(saveGen, (Isc - Imax) / Isc)
    Range(pmaxCell).Value = Pmax
    If Voc > 0 And Isc > 0 Then Range(ffCell).Value = Pmax / (Voc * Isc)
    rVoltage = Vmax: Range(voltageCell).Value = rVoltage
    Range(progressCell).Value = "Pmax": DoEvents
    Call InitializeGammaPsi
    If SolveSurfaces Then
        Vmax = ContactAverage(holes, 1, nNodesX, 1, nNodesY, sPhi) - ContactAverage(electrons, 1, nNodesX, 1, nNodesY, sPhi)
        rVoltage = Vmax: Range(voltageCell).Value = rVoltage
        Pmax = Vmax * Imax: Range(pmaxCell).Value = Pmax
        dataIV(1, 1) = Vmax
        dataIV(1, 2) = Imax
        dataIV(1, 3) = Pmax
    End If
    Range(IVrange) = dataIV
    If Voc > 0 And Isc > 0 Then Range(ffCell).Value = Pmax / (Voc * Isc)
    If Pfront + Prear > 0 Then Range(efficiencyCell).Value = 100 * Pmax / (Pfront + Prear)
    Call RefreshCharts
Else
    If Not bBatch Then MsgBox ("Pseudo max-power point not found")
End If
End Sub

Sub MacroSunsVoc()
Dim i As Integer, nIncrements As Integer, n As Integer
Dim rSuns As Double, rJo1 As Double, rJsc As Double, Voc As Double, Isc As Double
Dim Vmax As Double, Imax As Double, Pmax As Double
Dim Pspectrum As Double, Pfront As Double, Prear As Double
Dim saveTotalGen As Double
Dim saveGen As Variant, dataIV As Variant
ReDim saveGen(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double

dataIV = Range(IVrange)
If currentPlot = "Layout" Or currentPlot = "Generation" Then currentPlot = "Excess Carriers"
Call SolveGeneration
saveGen = vGeneration
saveTotalGen = rTotalGeneration
Pfront = 0: Prear = 0
If bSpectrum Then
    Pspectrum = 0
    For n = 1 To UBound(rSpectrumData, 1)
        Pspectrum = Pspectrum + rSpectrumData(n, 2) / 10000
    Next n
    Pfront = Pfront + Pspectrum * rFrontSuns * SurfaceAverage(1, nNodesX, 1, nNodesY, sFrontIllumination)
    Prear = Prear + Pspectrum * rRearSuns * SurfaceAverage(1, nNodesX, 1, nNodesY, sRearIllumination)
    Range(illuminationPowerCell).Value = Pfront + Prear
End If
If bMono Then
    Pfront = Pfront + rFrontMonoPower * SurfaceAverage(1, nNodesX, 1, nNodesY, sFrontIllumination)
    Prear = Prear + rRearMonoPower * SurfaceAverage(1, nNodesX, 1, nNodesY, sRearIllumination)
    Range(illuminationPowerCell).Value = Pfront + Prear
End If
If Pfront + Prear = 0 Then
    If Not bBatch Then MsgBox ("No illumination!")
    Range(progressCell).Value = "Null"
    Exit Sub
End If
If Pfront <> Pspectrum Or Prear <> 0 Or Not bSpectrum Then
    Range(ivWarningCell).Value = "Non-standard illumination"
Else
    If Pfront > 0 And bFilter Then Range(ivWarningCell).Value = "Filtered illumination"
End If

rJsc = q * rTotalGeneration
Voc = VocEstimate(rTotalGeneration)
rJo1 = rJsc / (Exp(Voc / rVt) - 1)
nIncrements = Range(voltageIncrementsCell)
dataIV(nIncrements + 1, 1) = Voc
Range(IVrange) = dataIV
Call RefreshCharts
bOpenCircuit = True: Range(openCircuitCell).Value = "Voc"

For i = 1 To nIncrements
    rSuns = Exp(-10 * (nIncrements - i) / nIncrements)
    vGeneration = MScale3D(saveGen, rSuns)
    rTotalGeneration = rSuns * saveTotalGen
    Voc = rVt * Log(1 + rJsc * rSuns / rJo1)
    rVoltage = Voc: Range(voltageCell).Value = rVoltage
    Range(progressCell).Value = "Suns-step": DoEvents
    Call InitializeGammaPsi
    If SolveSurfaces Then
        Voc = ContactAverage(holes, 1, nNodesX, 1, nNodesY, sPhi) - ContactAverage(electrons, 1, nNodesX, 1, nNodesY, sPhi)
        dataIV(i + 1, 1) = Voc
        dataIV(i + 1, 2) = rJsc * rSuns
    End If
    Range(IVrange) = dataIV
    Call RefreshCharts
Next i
End Sub

Sub MacroEQE()
Dim saveView As Variant, saveLastView As Variant, saveMode As Variant
If Not bDebug Then On Error GoTo ErrHandler:
If bNeedReset Then Call ResetAll
bScan = True
Call ResetGeneration
Call ResetElectrical
Call ClearEQE
Call RefreshCharts
saveView = Range(viewCell)
saveLastView = Range(lastPlotViewCell)
saveMode = Range(openCircuitCell)
Range(viewCell).Value = "Face"
Range(lastPlotViewCell).Value = "Face"
If Range(eqeFastCell) = "Fast" Then Call MacroFastEQE Else Call MacroSlowEQE
bScan = False
If Not bBatch Then Call OnResetAll       'The final countour plot is confusing so better to wipe it clean
Range(viewCell).Value = saveView
Range(lastPlotViewCell).Value = saveLastView
Range(openCircuitCell).Value = saveMode
Exit Sub
    
ErrHandler:
    bScan = False
    Range(progressCell) = "Error"
    Range(viewCell).Value = saveView
    Range(lastPlotViewCell).Value = saveLastView
    Range(openCircuitCell).Value = saveMode
End Sub

Sub MacroFastEQE()
'Find EQE by convolving collection efficiency (from dark excess pn product) with generation profile.
'This does not account for nonlinear recombination effects (n=2, high-level injection, etc.).
'Light bias is ignored.
Dim i As Integer, j As Integer, k As Integer, m As Integer, n As Integer, nSteps As Integer
Dim nLambdaMin As Integer, nLambdaMax As Integer
Dim lambda As Double, alpha As Double, current As Double, filter As Double
Dim minLambda As Double, maxLambda As Double
Dim saveVoltage As Double, localCurrent As Double
Dim frontSide As Boolean, rearSide As Boolean
Dim dataEQE As Variant
Dim frontPower As Double, rearPower As Double
Dim illuminationFlux As Double, photoGeneration As Double
Dim maxCollEff As Double, totalCurrent As Double
Dim vCollEff As Variant
ReDim vCollEff(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
Const smallPower As Double = 0.000001
dataEQE = Range(eqeRange)
frontSide = Range(eqeSideCell) = "Front"
rearSide = Range(eqeSideCell) = "Rear"
If (frontSide And frontShape <> "Uniform") Or (rearSide And rearShape <> "Uniform") Then
    Range(eqeWarningCell).Value = "Non-uniform illumination"
Else
    If frontSide And bFilter Then Range(eqeWarningCell).Value = "Filtered illumination"
End If
minLambda = Range(minLambdaCell)
maxLambda = Range(maxLambdaCell)
Worksheets("Dashboard").ChartObjects(eqePlot).Chart.Axes(xlCategory).MinimumScale = minLambda
Worksheets("Dashboard").ChartObjects(eqePlot).Chart.Axes(xlCategory).MaximumScale = maxLambda
nSteps = Range(eqeIncrementsCell) + 1
saveVoltage = rVoltage

'Forward bias in the dark to find collection efficiency.
'Bias voltage selected to roughly approximate peak carrier densities under one-sun short-circuit conditions in a typical solar cell.
If b3D Then currentPlot = "Excess Carriers" Else currentPlot = "Generation"
bOpenCircuit = False: Range(openCircuitCell).Value = "Contact"
rVoltage = 20 * rVt: Range(voltageCell).Value = rVoltage
Range(progressCell).Value = "Dark Bias": DoEvents
Call InitializeGammaPsi
If Not SolveSurfaces Then Exit Sub
maxCollEff = 0
For i = 1 To nNodesX: For j = 1 To nNodesY: For k = 1 To nNodesZ
    vCollEff(i, j, k) = ((rEqP + vGamma(i, j, k)) * (rEqN + vGamma(i, j, k)) - rNi ^ 2)
    If vCollEff(i, j, k) > maxCollEff Then maxCollEff = vCollEff(i, j, k)
Next k: Next j: Next i
For i = 1 To nNodesX: For j = 1 To nNodesY: For k = 1 To nNodesZ
    If maxCollEff > 0 Then vCollEff(i, j, k) = vCollEff(i, j, k) / maxCollEff
Next k: Next j: Next i

'Now apply monochromatic illumination of desired spatial distribution at each desired wavelength
bSpectrum = False
bMono = True
Call CalculateIlluminationUniformity(front, sFrontIllumination)
Call CalculateIlluminationUniformity(rear, sRearIllumination)
For n = 1 To nSteps
    lambda = minLambda + (maxLambda - minLambda) * (n - 1) / (nSteps - 1)
    Range(progressCell).Value = lambda: DoEvents
    alpha = InterpolateColumns(lambda, 1, rSpectrumData, 3, rSpectrumData, True, False)
    filter = 0.01 * InterpolateColumns(lambda, 1, rSpectrumData, 4, rSpectrumData, False, False)
    If frontSide Then frontPower = smallPower
    If rearSide Then rearPower = smallPower
    illuminationFlux = _
        frontPower * SurfaceAverage(1, nNodesX, 1, nNodesY, sFrontIllumination) * lambda / (q * hc) _
        + rearPower * SurfaceAverage(1, nNodesX, 1, nNodesY, sRearIllumination) * lambda / (q * hc)
    dataEQE(n, 1) = lambda
    Call CalculatePhotogeneration(lambda, alpha, frontPower, rearPower, sFrontIllumination, sRearIllumination, photoGeneration, vGeneration)
    If photoGeneration > 0 Then
        bGeneration = True
        Call PlotGeneration
        Range(generationCell).Value = q * photoGeneration  'A/cm2
    End If
    current = 0
    For i = 1 To nNodesX: For j = 1 To nNodesY: For k = 1 To nNodesZ
        localCurrent = q * vGeneration(i, j, k) * vCollEff(i, j, k)
        If i = 1 Or i = nNodesX Then localCurrent = 0.5 * localCurrent
        If j = 1 Or j = nNodesY Then localCurrent = 0.5 * localCurrent
        If k = 1 Or k = nNodesZ Then localCurrent = 0.5 * localCurrent
        current = current + localCurrent * deviceHeight / ((nNodesX - 1) * (nNodesY - 1) * (nNodesZ - 1))
    Next k: Next j: Next i
    If illuminationFlux > 0 Then dataEQE(n, 2) = 100 * filter * current / (q * illuminationFlux)
    If photoGeneration > 0 Then dataEQE(n, 3) = 100 * current / (q * photoGeneration)
    Range(eqeRange) = dataEQE
    Call RefreshCharts
Next n
'Integrate EQE weighted by Spectrum
'Find min and max lambda entries in rSpectrumData. Assumes increasing lambda, but may not fill array space.
nLambdaMin = LBound(rSpectrumData, 1)
nLambdaMax = nLambdaMin
For n = 1 To UBound(rSpectrumData)
    If rSpectrumData(n, 1) > rSpectrumData(nLambdaMax, 1) Then nLambdaMax = n
Next n
totalCurrent = 0
For n = nLambdaMin To nLambdaMax
    lambda = rSpectrumData(n, 1)
    If frontSide Then frontPower = rSpectrumData(n, 2) / 10000 'W/cm2
    If rearSide Then rearPower = rSpectrumData(n, 2) / 10000  'W/cm2
    illuminationFlux = _
        frontPower * SurfaceAverage(1, nNodesX, 1, nNodesY, sFrontIllumination) * lambda / (q * hc) _
        + rearPower * SurfaceAverage(1, nNodesX, 1, nNodesY, sRearIllumination) * lambda / (q * hc)
    totalCurrent = totalCurrent + q * illuminationFlux * InterpolateColumns(lambda, 1, dataEQE, 2, dataEQE, False, True) / 100
Next n
Range(eqeTotalCurrentCell).Value = totalCurrent
rVoltage = saveVoltage: Range(voltageCell).Value = rVoltage
End Sub

Sub MacroSlowEQE()
'Complete calculation of EQE including non-linear effects, including allowance for Spectrum light bias.
Dim i As Integer, j As Integer, m As Integer, n As Integer, nSteps As Integer
Dim saveFrontShape As String, saveRearShape As String
Dim nLambdaMin As Integer, nLambdaMax As Integer
Dim lambda As Double, alpha As Double, current As Double, biasJsc As Double, filter As Double
Dim minLambda As Double, maxLambda As Double
Dim saveVoltage As Double
Dim frontSide As Boolean, rearSide As Boolean
Dim dataEQE As Variant
Dim frontPower As Double, rearPower As Double, frontFlux As Double, rearFlux As Double
Dim illuminationFlux As Double, photoGeneration As Double, monoGeneration As Double
Dim biasSuns As Double, totalCurrent As Double
Dim biasGeneration As Variant
ReDim biasGeneration(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
Const smallPower As Double = 0.001 'W/cm2 for mono calculation, only important when biasSuns = 0
dataEQE = Range(eqeRange)
frontSide = Range(eqeSideCell) = "Front"
rearSide = Range(eqeSideCell) = "Rear"
If (frontSide And frontShape <> "Uniform") Or (rearSide And rearShape <> "Uniform") Then
    Range(eqeWarningCell).Value = "Non-uniform illumination"
Else
    If frontSide And bFilter Then Range(eqeWarningCell).Value = "Filtered illumination"
End If
minLambda = Range(minLambdaCell)
maxLambda = Range(maxLambdaCell)
Worksheets("Dashboard").ChartObjects(eqePlot).Chart.Axes(xlCategory).MinimumScale = minLambda
Worksheets("Dashboard").ChartObjects(eqePlot).Chart.Axes(xlCategory).MaximumScale = maxLambda
nSteps = Range(eqeIncrementsCell) + 1
biasSuns = Range(eqeBiasCell)
saveVoltage = rVoltage

bOpenCircuit = False: Range(openCircuitCell).Value = "Contact"
rVoltage = 0: Range(voltageCell).Value = rVoltage
If b3D Then currentPlot = "Excess Carriers" Else currentPlot = "Generation"
If biasSuns > 0 Then
    bSpectrum = True
    bMono = False
    If frontSide Then
        rFrontSuns = biasSuns
        rRearSuns = 0
        Call SolveGeneration        'Sets rTotalGeneration as bias generation rate
    Else    'rearSide
        rFrontSuns = 0
        rRearSuns = biasSuns
        Call SolveGeneration        'Sets rTotalGeneration as bias generation rate
    End If
    biasGeneration = vGeneration
    Range(progressCell).Value = "Light Bias": DoEvents
    Call InitializeGammaPsi
    If Not SolveSurfaces Then Exit Sub
    biasJsc = Range(terminalCurrentCell)
Else
    bSpectrum = False
    bMono = True
    rTotalGeneration = 0
    biasJsc = 0
    Call CalculateIlluminationUniformity(front, sFrontIllumination)
    Call CalculateIlluminationUniformity(rear, sRearIllumination)
End If

For n = 1 To nSteps
    lambda = minLambda + (maxLambda - minLambda) * (n - 1) / (nSteps - 1)
    Range(progressCell).Value = lambda: DoEvents
    alpha = InterpolateColumns(lambda, 1, rSpectrumData, 3, rSpectrumData, True, False)
    filter = 0.01 * InterpolateColumns(lambda, 1, rSpectrumData, 4, rSpectrumData, False, False)
    If frontSide Then frontPower = smallPower
    If rearSide Then rearPower = smallPower
    illuminationFlux = _
        frontPower * SurfaceAverage(1, nNodesX, 1, nNodesY, sFrontIllumination) * lambda / (q * hc) _
        + rearPower * SurfaceAverage(1, nNodesX, 1, nNodesY, sRearIllumination) * lambda / (q * hc)
    dataEQE(n, 1) = lambda
    Call CalculatePhotogeneration(lambda, alpha, frontPower, rearPower, sFrontIllumination, sRearIllumination, photoGeneration, vGeneration)
    If photoGeneration > 0 Then
        bGeneration = True
        Call PlotGeneration
        If rTotalGeneration > 0 Then    'Scale mono photogeneration to 1% of bias-light photogeneration
            monoGeneration = 0.01 * rTotalGeneration
            vGeneration = MScale3D(vGeneration, monoGeneration / photoGeneration)
            illuminationFlux = illuminationFlux * monoGeneration / photoGeneration
            photoGeneration = monoGeneration
            vGeneration = MAdd3D(vGeneration, biasGeneration)
        End If
        Range(generationCell).Value = q * (photoGeneration + rTotalGeneration) 'A/cm2
        If b3D Then currentPlot = "Excess Carriers"
        Call InitializeGammaPsi
        If Not SolveSurfaces Then Exit Sub
    End If
    current = Range(terminalCurrentCell) - biasJsc
    If illuminationFlux > 0 Then dataEQE(n, 2) = 100 * filter * current / (q * illuminationFlux)
    If photoGeneration > 0 Then dataEQE(n, 3) = 100 * current / (q * photoGeneration)
    Range(eqeRange) = dataEQE
    Call RefreshCharts
Next n
'Integrate EQE weighted by Spectrum
'Find min and max lambda entries in rSpectrumData. Assumes increasing lambda, but may not fill array space.
nLambdaMin = LBound(rSpectrumData, 1)
nLambdaMax = nLambdaMin
For n = 1 To UBound(rSpectrumData)
    If rSpectrumData(n, 1) > rSpectrumData(nLambdaMax, 1) Then nLambdaMax = n
Next n
totalCurrent = 0
For n = nLambdaMin To nLambdaMax
    lambda = rSpectrumData(n, 1)
    If frontSide Then frontPower = rSpectrumData(n, 2) / 10000 'W/cm2
    If rearSide Then rearPower = rSpectrumData(n, 2) / 10000  'W/cm2
    illuminationFlux = _
        frontPower * SurfaceAverage(1, nNodesX, 1, nNodesY, sFrontIllumination) * lambda / (q * hc) _
        + rearPower * SurfaceAverage(1, nNodesX, 1, nNodesY, sRearIllumination) * lambda / (q * hc)
    totalCurrent = totalCurrent + q * illuminationFlux * InterpolateColumns(lambda, 1, dataEQE, 2, dataEQE, False, True) / 100
Next n
Range(eqeTotalCurrentCell).Value = totalCurrent
rVoltage = saveVoltage: Range(voltageCell).Value = rVoltage
End Sub

Sub AdjustGraphAxes()
If Range(ivTypeCell) = "Light IV" Then _
    Worksheets("Dashboard").ChartObjects(ivPlot).Chart.Axes(xlValue).ScaleType = xlScaleLinear
If Range(ivTypeCell) = "Dark IV" Then _
    Worksheets("Dashboard").ChartObjects(ivPlot).Chart.Axes(xlValue).ScaleType = xlScaleLogarithmic
If Range(ivTypeCell) = "Pseudo IV" Then _
    Worksheets("Dashboard").ChartObjects(ivPlot).Chart.Axes(xlValue).ScaleType = xlScaleLinear
If Range(ivTypeCell) = "SunsVoc" Then _
    Worksheets("Dashboard").ChartObjects(ivPlot).Chart.Axes(xlValue).ScaleType = xlScaleLogarithmic
Call RefreshCharts
End Sub

Sub ClearIV()
Range(IVrange).ClearContents
Range(iscCell).Value = ""
Range(vocCell).Value = ""
Range(pmaxCell).Value = ""
Range(ffCell).Value = ""
Range(illuminationPowerCell).Value = ""
Range(efficiencyCell).Value = ""
Range(ivWarningCell).Value = ""
Range(terminalVoltageCell).Value = ""
Range(terminalCurrentCell).Value = ""
Range(terminalPowerCell).Value = ""
Call UpdateCircuitModel
Call AdjustGraphAxes
End Sub

Sub ClearEQE()
Range(eqeRange).ClearContents
Range(eqeTotalCurrentCell).Value = ""
Range(eqeWarningCell).Value = ""
Range(terminalVoltageCell).Value = ""
Range(terminalCurrentCell).Value = ""
Range(terminalPowerCell).Value = ""
DoEvents
End Sub

