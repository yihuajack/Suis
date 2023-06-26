Attribute VB_Name = "GlobalVariables"
Option Explicit
'This module defines variable names that are accessible in all modules
'and provides a subroutine for reseting all values to "unsolved" default values
'Array names that describe surfaces start with s. Array names for volume variables start with v.
'2D Arrays are base 1 to be compatible with Excel built-in matrix operations.
'Integer names start with n, real (double) names start with r, d, or f except fundamental constants like q, pi, and hc.
'Boolean values start with b. String names start with any small letter. Course perturbation values start with c.
'Locally defined variable names may not comply with the above constraints, but always start with a small letter.
'Subroutines and functions always start with a capital letter.

'Constants
#If Mac Then
    Public Const bMac As Boolean = True
#Else
    Public Const bMac As Boolean = False
#End If
Public Const bDebug As Boolean = False            'Set True to disable error handling for debugging purposes
'Defined Constants
Public Const ptype As Integer = 1, ntype As Integer = 2
Public Const holes As Integer = 1, electrons As Integer = 2, psi As Integer = 3, combined = 4
Public Const xAxis As Integer = 1, yAxis As Integer = 2, zAxis As Integer = 3, vector As Integer = 4
Public Const point As Integer = 1, line As Integer = 2, plane As Integer = 3  'Defect type
Public Const gaussian As Integer = 1, erfc As Integer = 2, none As Integer = 3     'Doping shape
Public Const planar As Integer = 1, grooves As Integer = 2, pyramids As Integer = 3, inverted As Integer = 4    'Texture types
Public Const bipolar As Integer = 1, floating As Integer = 2, opencircuit As Integer = 3, flatband As Integer = 4, lateral As Integer = 5   'Solution modes
Public Const TE As Integer = 1, TM As Integer = 2       'Polarization modes
Public Const nMaxRes1D As Integer = 100
Public Const nMaxRes2D As Integer = 80
Public Const nMaxRes3D As Integer = 60
Public Const nSelectDirection As Integer = vector   'Make vector plots show a single axis by changing this value
Public Const nPlotHarmonics As Integer = 40
Public Const nPlotNodes As Integer = nPlotHarmonics + 1
Public Const nArrowNodes As Integer = 10
Public Const nColumns As Integer = 5
Public Const nRows As Integer = 5
Public Const nGrid As Integer = 10   'Sampling grid for nodal surface averaging
Public Const pi As Double = 3.141592654
Public Const rQ As Double = 1.6021773E-19    'coulombs = ampere-sec
Public Const rEps0 As Double = 8.85418782E-14       'permittivity of free space
Public Const hc As Double = 1239.8424   'eV-nm
Public Const rVt300 As Double = 0.025851483     'volts at 300 kelvin
Public Const rToffset As Double = 273.15     'degrees kelvin
'Problem Definition Variables
Public nDimensions As Integer       '1, 2, or 3 dimensional solution
Public nHarmonicsX As Integer, nHarmonicsY As Integer, nHarmonicsZ As Integer
Public nNodesX As Integer, nNodesY As Integer, nNodesZ As Integer       'Long may be needed if X*Y*Z is calculated
Public nMaxIterations As Integer
Public nSolutionMode As Integer
Public nDiffusionType As Integer
Public nDiffusionShape As Integer
Public nSurfaceType As Integer
Public nTextureType As Integer
Public rTemperature As Double           'kelvin
Public rVoltage As Double, rVt As Double        'volts
Public textureAngle As Double, textureDepth As Double, baseHeight As Double     'um
Public deviceWidth As Double, deviceLength As Double, deviceHeight As Double     'cm
Public rBackgroundDoping As Double  'cm-3
Public nBackgroundType As Integer
Public rBackgroundPsi As Double     'volts
Public rBackgroundP As Double, rBackgroundN As Double       'cm-3
Public rBackgroundDp As Double, rBackgroundDn As Double     'cm2/s
Public rDiffusionPeak As Double     'cm-3
Public rDiffusionDepth As Double      'cm
Public rTauN As Double, rTauP As Double         'seconds (background)
Public rNi25C As Double, rNi As Double        'cm-3   prior to BGN
Public rEps As Double   'Relative permittivity of material
Public rQ_Eps As Double 'rQ/epsilon
Public rFrontSpeak As Double, rFrontSvalley As Double           'cm/s
Public rFrontQpeak As Double, rFrontQvalley As Double           'cm-2
Public rCn As Double, rCp As Double       'cm-6/s
Public rVpSlope As Double, rVnSlope As Double   'mV
Public rVpOnset As Double, rVnOnset As Double   'cm-3
Public rFrontSuns As Double, rEncapsulationIndex As Double
Public rMonoLambda As Double, rFrontMonoPower As Double       'nm, W/cm2
Public rDefectPosition As Variant
Public lineDefectDirection As String, planeDefectDirection As String
Public rDefectTauN As Double, rDefectTauP As Double, rDefectBGN As Double
Public rLambdaAMg As Variant
Public rPowerAMg As Variant
Public rSiAlpha As Variant
Public rSiIndex As Variant
Public rSpectrumData As Variant, rCoatingData As Variant
Public rSpectralLambdas As Variant, rSpectralFront As Variant, rSpectralRear As Variant
'Boodean global values
Public bEnablePointDefect As Boolean, bEnableLineDefect As Boolean, bEnablePlaneDefect As Boolean
Public bIllumination As Boolean, bSpectrum As Boolean, bMono As Boolean
Public bNeedReset As Boolean, bEnableChangeHandlers As Boolean
Public bScan As Boolean, bBatch As Boolean, bStartup As Boolean, bImage As Boolean
Public bConvergenceFailure As Boolean
Public bVolumeScale As Boolean
'Real global values
Public rConvergence As Double, rDamping As Double
Public rStartTime As Double
Public rTotalGeneration As Double, rTotalRecombination As Double      'cm-2/s
Public rBulkRecombination As Double, rSurfaceRecombination As Double        'cm-2/s
Public rLateralCurrent As Double        'A
Public rCosTextureAngle As Double, rSinTextureAngle As Double
Public rDeltaX As Double, rDeltaY As Double, rDeltaZ As Double     'cm
Public rDeltaX2 As Double, rDeltaY2 As Double, rDeltaZ2 As Double      'cm2
Public k2x As Double, k2y As Double, k2z As Double, k2xyz As Double 'cm-2
'2D (surface) variables
Public sTexture As Variant
Public sCharge As Variant      'cm-2
Public sVelocity As Variant     'cm/s
Public sRecombination As Variant        'cm-2/s
'3D (volume) variables
Public vDopingP As Variant, vDopingN As Variant           'cm-3
Public vVp As Variant, vVn As Variant  'volts of bandgap narrowing
Public vPhiP As Variant, vPhiN As Variant, vPsi As Variant      'volts
Public vDp As Variant, vDn As Variant       'cm2/s
Public vSqrSigmaP As Variant, vSqrSigmaN As Variant       'S/cm
Public vGeneration As Variant, vRecombination As Variant        'cm-3/s
Public vGammaP As Variant, vGammaN As Variant            'A/cm
Public vGradGammaP As Variant, vGradGammaN As Variant        'A/cm2
Public vectorArray As Variant       'Last plotted vector data
'Transforms (depend only on geometry and resolution)
Public kmnp2 As Variant
Public f2rX As Variant, f2rY As Variant, f2rZ As Variant        'Fourier-to-Real used for harmonic representation of potentials
Public r2fX As Variant, r2fY As Variant, r2fZ As Variant        'Real-to-Fouier used to harmonic represtation of potentials
Public f2gX As Variant, f2gY As Variant, f2gZ As Variant        'Fourier-to-Gradient used to calculate harmonic gradients
Public r2tX As Variant, r2tY As Variant, r2tZ As Variant        'Functional to real for conversion to nPlotNodes-1 resolution
Public r2aX As Variant, r2aY As Variant, r2aZ As Variant        'Functional to real for display of vector arrows
'Output variables
Public sPlot As Variant
Public currentPlot As String
Public nSelectCarrier As Integer
Public bIntermediatePlot As Boolean

'These are spreadsheet cell locations that are read or modified by the code.
'Any re-arrangement of cells on the spreadsheets requires a corresponding change to these constants.
'Most of them are used in the FileHandling module to save and load parameters.
'DASHBOARD
Public Const eqePlot = "Chart 28"
Public Const contourPlot = "Chart 29"
Public Const programNameCell = "Dashboard!J1"
Public Const programVersionCell = "Dashboard!J6"
Public Const dimensionsCell = "Dashboard!C3"
Public Const loadedFileCell = "Dashboard!C1"
Public Const resolutionCell = "Dashboard!D3"
Public Const XharmonicsCell = "Dashboard!B23"
Public Const YharmonicsCell = "Dashboard!B24"
Public Const ZharmonicsCell = "Dashboard!B25"
Public Const deltaPCell = "Dashboard!D23"
Public Const deltaNCell = "Dashboard!D24"
Public Const deltaPsiCell = "Dashboard!D25"
Public Const contactModeCell = "Dashboard!D2"
Public Const contactUnitsCell = "Dashboard!H1"
Public Const convergenceCell = "Dashboard!D4"
Public Const errorCell = "Dashboard!D5"
Public Const progressCell = "Dashboard!D7"
Public Const iterationCell = "Dashboard!D6"
Public Const voltageCell = "Dashboard!G1"
Public Const illuminationCell = "Dashboard!G2"
Public Const maxIterationsCell = "Dashboard!B20"
Public Const solutionTimeCell = "Dashboard!B21"
Public Const dampingCell = "Dashboard!D20"
Public Const thermalVoltageCell = "Dashboard!D21"
Public Const generationCell = "Dashboard!G3"
Public Const bulkRecombinationCell = "Dashboard!G4"
Public Const surfaceRecombinationCell = "Dashboard!G5"
Public Const totalRecombinationCell = "Dashboard!G6"
Public Const lateralCurrentCell = "Dashboard!G7"
Public Const macroJo1Cell = "Dashboard!B14"
Public Const macroJo2Cell = "Dashboard!B15"
Public Const macroGshCell = "Dashboard!B16"
Public Const macroSheetRhoCell = "Dashboard!E14"
Public Const spectralRange = "Dashboard!K11:L29"
Public Const transmissionRange = "Dashboard!L11:L29"
Public Const lastPlotViewCell = "Dashboard!R17"
Public Const viewCell = "Dashboard!O10"
Public Const positionCell = "Dashboard!Q10"
Public Const unitCell = "Dashboard!O11"
Public Const minCell = "Dashboard!Q11"
Public Const maxCell = "Dashboard!S11"
Public Const titleCell = "Dashboard!Q9"
Public Const selectCarrierCell = "Dashboard!R6"
Public Const plotChangeCells = viewCell & ", " & positionCell & ", " & selectCarrierCell
'DEVICE
Public Const deviceWidthCell = "Device!B2"
Public Const deviceLengthCell = "Device!E2"
Public Const deviceHeightCell = "Device!H2"
Public Const deviceAreaCell = "Device!K2"
Public Const deviceDescriptionCell = "Device!N2"
Public Const textureTypeCell = "Device!B5"
Public Const textureDepthCell = "Device!D5"
Public Const baseHeightCell = "Device!G5"
Public Const textureAngleCell = "Device!J5"
Public Const deviceDiffusionTypeCell = "Device!B8"
Public Const deviceDiffusionPeakCell = "Device!D8"
Public Const deviceDiffusionDepthCell = "Device!G8"
Public Const diffusionShapeCell = "Device!J8"
Public Const deviceDopingTypeCell = "Device!B10"
Public Const deviceDopingCell = "Device!D10"
Public Const resistivityCell = "Device!G10"
Public Const minorityCell = "Device!J10"
Public Const frontChargePeakCell = "Device!D13"
Public Const frontChargeValleyCell = "Device!H13"
Public Const frontChargeShapeCell = "Device!K13"
Public Const permittivityCell = "Device!O13"
Public Const frontSpeakCell = "Device!D16"
Public Const frontSvalleyCell = "Device!H16"
Public Const frontSshapeCell = "Device!K16"
Public Const intrinsic25Cell = "Device!G18"
Public Const intrinsicCell = "Device!G19"
Public Const tauNCell = "Device!C18"
Public Const tauPCell = "Device!C19"
Public Const augerPtypeCell = "Device!K18"
Public Const augerNtypeCell = "Device!K19"
Public Const temperatureCell = "Device!O16"
Public Const valenceBGNslopeCell = "Device!O20"
Public Const valenceBGNonsetCell = "Device!O21"
Public Const conductionBGNslopeCell = "Device!O23"
Public Const conductionBGNonsetCell = "Device!O24"
Public Const defectEnableCells = "Device!C22:C24"
Public Const lineDefectDirectionCell = "Device!D23"
Public Const planeDefectDirectionCell = "Device!D24"
Public Const defectPositionCells = "Device!E22:G24"
Public Const defectTauNCell = "Device!J22"
Public Const defectTauPCell = "Device!J23"
Public Const defectBGNCell = "Device!J24"
Public Const BGNCells = valenceBGNslopeCell & ", " & valenceBGNonsetCell & ", " & conductionBGNslopeCell & ", " & conductionBGNonsetCell
Public Const textureChangeCells = textureTypeCell & ", " & textureAngleCell & ", " & textureDepthCell & ", " & baseHeightCell
Public Const diffusionChangeCells = deviceDiffusionTypeCell & ", " & deviceDiffusionPeakCell & ", " & deviceDiffusionDepthCell
Public Const dopingChangeCells = deviceDopingCell & ", " & deviceDopingTypeCell
Public Const lifetimeValueCells = tauNCell & ", " & tauPCell & ", " & intrinsic25Cell & ", " & augerPtypeCell & ", " & augerNtypeCell
Public Const frontSCells = frontSpeakCell & ", " & frontSvalleyCell
Public Const frontChargeCells = frontChargePeakCell & ", " & frontChargeValleyCell
Public Const defectValueCells = defectPositionCells & ", " & defectTauNCell & ", " & defectTauPCell & ", " & defectBGNCell
Public Const defectDropdownCells = defectEnableCells & ", " & lineDefectDirectionCell & ", " & planeDefectDirectionCell
'ILLUMINATION
Public Const spectrumCell = "Optics!B2"
Public Const frontSunsCell = "Optics!C5"
Public Const frontPowerCell = "Optics!C8"
Public Const lambdaCell = "Optics!C9"
Public Const encapsulationIndexCell = "Optics!G10"
Public Const spectrumDataCells = "Optics!A14:D72"
Public Const coatingDataCells = "Optics!F3:H8"
'DATA
Public Const dataIVCells = "Data!A4:B6"
Public Const sheetRhoVoltageCell = "Data!C4"
Public Const sheetRhoCurrentCell = "Data!D4"
Public Const dataSpectralCells = "Data!A9:D27"
Public Const columnarHeaderCells = "Data!F1:H1"
Public Const tabularUnitsCell = "Data!K2"
Public Const plotNameCell = "Data!H1"
Public Const plotViewCell = "Data!I2"
Public Const plotPositionCell = "Data!I4"
Public Const plotUnitsCell = "Data!I6"
Public Const plotMinCell = "Data!I8"
Public Const plotMaxCell = "Data!I10"
Public Const plotResolutionCell = "Data!I12"
Public Const plotCarrierCell = "Data!I14"
Public Const plotDirectionCell = "Data!I22"
Public Const tabularAbscissaCells = "Data!L2:AZ2"
Public Const tabularOrdinateCells = "Data!K3:K43"
Public Const normalizedDataCells = "Data!L47:AZ87"
Public Const columnarDataCells = "Data!F2:H10252"   'Big enough for 200 x 50 resolution
Public Const tabularDataCells = "Data!L3:AZ43"
Public Const normalizedChartCells = "Data!K46:AZ87"
'BATCH
Public Const batchMaxIntervals = 25
Public Const batchMaxOutputValues = 10
Public Const batchRunningCell = "Batch!A2"
Public Const batchDescriptionCell = "Batch!C3"
Public Const batchStartCell = "Batch!A9"
Public Const batchEndCell = "Batch!A11"
Public Const batchIntervalsCell = "Batch!A13"
Public Const batchScaleCell = "Batch!A15"
Public Const batchInputNameCell = "Batch!C5"
Public Const batchOutputNameCells = "Batch!D5:M5"
Public Const batchInputDataCells = "Batch!C6:C31"
Public Const batchOutputDataCells = "Batch!D6:M31"

Sub InitializeAll()
'This routine must not change any spreadsheet values or Charts to maintain cross-platform compatibility.
'This routine does not set either bEnableChangeHandlers or bNeedReset.
Call MapInputParameters     'Fills arrays necessary to deterimine if 3D is required and dimensions that fit texture.
'Establish resolution in each axis, then allocate memory for spatial variables.
nDimensions = 1
If Requires2D Then nDimensions = 2    'Assess whether 2D is needed due to nonuniformity in the x-axis (width)
If Requires3D Then nDimensions = 3    'Assess whether 3D is needed due to nonuniformity in the y-axis (length)
If Range(resolutionCell) = "Max" Then
    Select Case nDimensions
    Case 1: nHarmonicsZ = nMaxRes1D
    Case 2: nHarmonicsZ = nMaxRes2D
    Case 3: nHarmonicsZ = nMaxRes3D
    End Select
Else
    nHarmonicsZ = Range(resolutionCell)
End If
nNodesZ = nHarmonicsZ + 1
Call CalculateTexture           'Sets nHarmonics, and nNodes for x and y axis to match requested texture angle
ReDim kmnp2(1 To nHarmonicsX + 1, 1 To nHarmonicsY + 1, 1 To nHarmonicsZ + 1) As Double
ReDim f2rX(1 To nNodesX, 1 To nHarmonicsX + 1) As Double
ReDim f2rY(1 To nNodesY, 1 To nHarmonicsY + 1) As Double
ReDim f2rZ(1 To nNodesZ, 1 To nHarmonicsZ + 1) As Double
ReDim r2fX(1 To nHarmonicsX + 1, 1 To nNodesX) As Double
ReDim r2fY(1 To nHarmonicsY + 1, 1 To nNodesY) As Double
ReDim r2fZ(1 To nHarmonicsZ + 1, 1 To nNodesZ) As Double
ReDim f2gX(1 To nNodesX, 1 To nHarmonicsX + 1) As Double
ReDim f2gY(1 To nNodesY, 1 To nHarmonicsY + 1) As Double
ReDim f2gZ(1 To nNodesZ, 1 To nHarmonicsZ + 1) As Double
ReDim r2tX(1 To nPlotNodes, 1 To nHarmonicsX + 1) As Double
ReDim r2tY(1 To nPlotNodes, 1 To nHarmonicsY + 1) As Double
ReDim r2tZ(1 To nPlotNodes, 1 To nHarmonicsZ + 1) As Double
ReDim r2aX(1 To nArrowNodes, 1 To nHarmonicsX + 1) As Double
ReDim r2aY(1 To nArrowNodes, 1 To nHarmonicsY + 1) As Double
ReDim r2aZ(1 To nArrowNodes, 1 To nHarmonicsZ + 1) As Double
ReDim sCharge(1 To nNodesX, 1 To nNodesY) As Double
ReDim sVelocity(1 To nNodesX, 1 To nNodesY) As Double
ReDim sRecombination(1 To nNodesX, 1 To nNodesY) As Double
ReDim sPlot(1 To nPlotNodes, 1 To nPlotNodes) As Double
ReDim vVp(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
ReDim vVn(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
ReDim vPhiP(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
ReDim vPhiN(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
ReDim vPsi(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
ReDim vSqrSigmaP(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
ReDim vSqrSigmaN(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
ReDim vDopingP(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
ReDim vDopingN(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
ReDim vDp(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
ReDim vDn(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
ReDim vGammaP(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
ReDim vGammaN(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
ReDim vGradGammaP(xAxis To zAxis, 1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
ReDim vGradGammaN(xAxis To zAxis, 1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
ReDim vRecombination(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
ReDim vGeneration(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
ReDim vectorArray(xAxis To zAxis, 1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
rLambdaAMg = Array(310#, 320#, 330#, 340#, 350#, 370#, 390#, 410#, 430#, 450#, 470#, 490#, 510#, 530#, 550#, 590#, 630#, 670#, 710#, 730#, 750#, 770#, 790#, 810#, 830#, 860#, 900#, 920#, 940#, 960#, 990#, 1070#, 1120#, 1140#, 1180#, 1220#, 1280#, 1360#, 1460#, 1540#, 1580#, 1610#, 1640#, 1740#, 1860#, 1960#, 2000#, 2060#, 2150#, 2270#, 2450#, 2540#, 2980#, 3060#, 3160#, 3240#, 3340#, 3580#, 4000#)
rSiAlpha = Array(1440000#, 1280000#, 1170000#, 1090000#, 1040000#, 697000#, 150000#, 67400#, 39200#, 25500#, 17200#, 12700#, 9700#, 7850#, 6390#, 4490#, 3270#, 2390#, 1780#, 1540#, 1300#, 1100#, 927#, 707#, 646#, 480#, 306#, 200#, 183#, 134#, 79.2, 8#, 2#, 1#, 0.065, 0.0082, 0.00012, 0.000001, 0.000001, 0.000001, 0.000001, 0.000001, 0.000001, 0.000001, 0.000001, 0.000001, 0.000001, 0.000001, 0.000001, 0.000001, 0.000001, 0.000001, 0.000001, 0.000001, 0.000001, 0.000001, 0.000001, 0.000001, 0.000001)
rPowerAMg = Array(0.72, 2.098, 4.137, 4.54, 7.8, 13.24, 14.554, 23.596, 23.964, 29.922, 31.471, 30.971, 30.34, 30.786, 46.055, 59.071, 57.249, 54.424, 37.318, 22.895, 23.689, 19.435, 22.38, 19.899, 23.96, 33.649, 23.915, 13.4, 5.945, 12.296, 39.5, 45.457, 5.609, 6.446, 17.074, 22.693, 27.044, 5.102, 7.075, 15.49, 8.718, 7.067, 14.372, 16.759, 0.149, 1.452, 2.875, 6.055, 8.367, 9.837, 3.682, 0.058, 0.478, 0.276, 0.611, 0.307, 0.644, 1.5, 1.584)
rSiIndex = Array(5.074, 5.102, 5.179, 5.293, 5.483, 6.863, 5.976, 5.305, 4.925, 4.676, 4.491, 4.348, 4.243, 4.151, 4.077, 3.963, 3.878, 3.815, 3.764, 3.743, 3.723, 3.706, 3.689, 3.675, 3.662, 3.644, 3.62, 3.609, 3.598, 3.588, 3.574, 3.549, 3.537, 3.532, 3.524, 3.516, 3.507, 3.496, 3.487, 3.482, 3.479, 3.477, 3.476, 3.469, 3.461, 3.455, 3.452, 3.448, 3.443, 3.435, 3.423, 3.42, 3.419, 3.419, 3.418, 3.418, 3.418, 3.417, 3.416)
rTotalGeneration = 0: rTotalRecombination = 0: rLateralCurrent = 0: rBulkRecombination = 0: rSurfaceRecombination = 0
bVolumeScale = False: bIntermediatePlot = False
bConvergenceFailure = False
bScan = False: bBatch = False: bImage = False
End Sub

Sub ResetAll()
'Initializes all global variables set by programming.
'Calls routines that map input values from spreadsheet, clears some spreadsheet cells, then plots Layout.
'Establishes node count in each axis and maps surface parameters onto these nodes.
'ResetAll, LoadFile, and ValidatePosition are the only user routines that set/reset the change handlers directly.
bStartup = False        'Once reset, the program is no longer in the startup mode

Application.Calculation = xlCalculationManual
bEnableChangeHandlers = False
Range(progressCell).Value = "Reset": DoEvents
Range(batchRunningCell).Value = ""
Call InitializeAll
Range(XharmonicsCell) = nHarmonicsX
Range(YharmonicsCell) = nHarmonicsY
Range(ZharmonicsCell) = nHarmonicsZ
Select Case nDimensions
    Case 1: Range(dimensionsCell).Value = "Resolution (1D):"
    Case 2: Range(dimensionsCell).Value = "Resolution (2D):"
    Case 3: Range(dimensionsCell).Value = "Resolution (3D):"
End Select
Call OnTemperatureChange     'Adjusts temperature-dependent values
Call OnDopingChange         'Adjusts doping-dependent cells
Call OnShapeChange      'Adjusts shape-dependent cell formating
bNeedReset = False  'Must come after the above calls, which set this to True
Call ResetOutputs
Call CalculateTexture
Call CalculateTransforms
Call CalculateDoping
Worksheets("Dashboard").ChartObjects(contourPlot).Chart.SetSourceData Source:=Sheets("Data").Range(normalizedChartCells)
Worksheets("Dashboard").ChartObjects(contourPlot).Chart.ChartGroups(1).Has3DShading = False
currentPlot = Range(titleCell)
Call OnPlotChange
Range(progressCell).Value = "" 'Clear progress field to indicate Reset is successfully finished
Application.Calculation = xlCalculationAutomatic
bEnableChangeHandlers = True
End Sub

Sub ResetGeneration()
'This routine reinitializes the photogeneration in preparation for a new optical solution.
ReDim vGeneration(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
rTotalGeneration = 0
End Sub

Sub ResetElectrical()
'This routine reinitializes all electrical values in preparation for a new electrical solution.
ReDim vPhiP(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
ReDim vPhiN(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
ReDim vPsi(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
ReDim vGammaP(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
ReDim vGammaN(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
ReDim vGradGammaP(xAxis To zAxis, 1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
ReDim vGradGammaN(xAxis To zAxis, 1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
ReDim vSqrSigmaP(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
ReDim vSqrSigmaN(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
ReDim vRecombination(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
ReDim sRecombination(1 To nNodesX, 1 To nNodesY) As Double
ReDim vectorArray(xAxis To zAxis, 1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
rTotalRecombination = 0: rBulkRecombination = 0: rSurfaceRecombination = 0
rLateralCurrent = 0
End Sub

Sub ResetOutputs()
Range(errorCell).Value = ""
Range(iterationCell).Value = ""
Range(generationCell).Value = ""
Range(bulkRecombinationCell).Value = ""
Range(surfaceRecombinationCell).Value = ""
Range(totalRecombinationCell).Value = ""
Range(lateralCurrentCell).Value = ""
Range(solutionTimeCell) = ""
Range(deltaPCell).Value = ""
Range(deltaNCell).Value = ""
Range(deltaPsiCell).Value = ""
End Sub

