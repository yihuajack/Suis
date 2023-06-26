Attribute VB_Name = "GlobalVariables"
Option Explicit
'This module defines variable names that are accessible in all modules
'and provides a subroutine for reseting all values to "unsolved" default values
'Array names that describe surfaces start with s. Array names for volume variables start with v.
'2D and 3D Arrays are base 1 to be compatible with Excel built-in matrix operations.
'Integer names start with n, real (double) names start with r, d, or f except fundamental constants like q, pi, and hc.
'Boolean values start with b. String names start with any small letter. Regional element arrays start with e.
'Locally defined variable names may not comply with the above constraints, but always start with a small letter.
'Subroutines and functions always start with a capital letter.
#If Mac Then
    Public Const bMac As Boolean = True
#Else
    Public Const bMac As Boolean = False
#End If
Public Const bDebug = False             'Set True to disable error handling for debugging purposes
Public Const nMaxIterations = 30
Public Const nMaxNodes = 41
Public Const nFlyThroughSteps = 50
Public Const nColumns As Integer = 5
Public Const nRows As Integer = 5
Public Const nGrid As Integer = 10   'Sampling grid for nodal region averaging
Public Const nElements As Integer = 3   'Element nodes are coarser than solution nodes to limit Jacobian size/time
Public Const nElementNodes As Integer = nElements + 1
Public Const nRegions As Integer = nElementNodes      'A region is the area surrounding each element node
Public Const Pi As Double = 3.141592654
Public Const q As Double = 1.6021773E-19    'coulombs = ampere-sec
Public Const hc As Double = 1239.8424   'eV-nm
Public Const rVt300 As Double = 0.025851483     'volts at 300 kelvin
Public Const rToffset = 273.15      'degrees kelvin
Public Const front As Integer = 1, rear As Integer = 2
Public Const holes As Integer = 1, electrons As Integer = 2
Public Const gamma As Integer = 1, psi As Integer = 2
Public Const TR = 1, TL = 2, BL = 3, BR = 4
Public Const xAxis = 1, yAxis = 2, zAxis = 3
Public Const point = 1, line = 2, plane = 3     'Defect type
Public nHarmonicsX As Integer, nHarmonicsY As Integer, nHarmonicsZ As Integer
Public nNodesX As Long, nNodesY As Long, nNodesZ As Long    'Long is necessary because 41*41*41 overflows integer
Public rTemperature As Double           'kelvin
Public rVoltage As Double, rVt As Double        'volts
Public deviceWidth As Double, deviceLength As Double, deviceHeight As Double     'cm
Public rConvergence As Double
Public rDoping As Double, rPsiEq As Double, rMinEq As Double        'cm-3
Public rEqN As Double, rEqP As Double            'cm-3
Public rDp As Double, rDn As Double, rDmin As Double, rDamb As Double     'cm2/s
Public rSeries As Double                          'ohm-cm2
Public rTauN As Double, rTauP As Double         'seconds
Public rNi As Double, rCa As Double, rInvL2 As Double       'cm-3, cm6/s, cm-2
Public rFrontSuns As Double, rRearSuns As Double
Public rMonoLambda As Double, rFrontMonoPower As Double, rRearMonoPower As Double       'nm, W/cm2
Public rFrontContacts As Variant, rRearContacts As Variant
Public rFrontSheetRho As Variant, rRearSheetRho As Variant
Public rFrontTrans As Variant, rRearTrans As Variant
Public rFrontHaze As Variant, rRearHaze As Variant
Public rFrontSpecRef As Variant, rRearSpecRef As Variant
Public rFrontDiffRef As Variant, rRearDiffRef As Variant
Public rFrontJo1 As Variant, rRearJo1 As Variant
Public rFrontJo2 As Variant, rRearJo2 As Variant
Public rFrontGsh As Variant, rRearGsh As Variant
Public rDefectPosition As Variant
Public vPointRecombination As Variant, vLineRecombination As Variant, vPlaneRecombination As Variant
Public rPointDamping As Double, rLineDamping As Double, rPlaneDamping As Double
Public rDeltaGammaDeltaR As Double
Public nElementIDx As Variant, nElementIDy   'Identifies in which element each node lies, for both surfaces
Public elementBoundariesX As Variant, elementBoundariesY As Variant 'Upper element boundaries in mm from origin
Public eRegionLowerX As Variant, eRegionUpperX As Variant   'Range of nodes that lie within the region around each element node
Public eRegionLowerY As Variant, eRegionUpperY As Variant
Public eElementLowerX As Variant, eElementUpperX As Variant 'Range of nodes in each element
Public eElementLowerY As Variant, eElementUpperY As Variant
Public eNodeAllocationX As Variant, eNodeAllocationY As Variant 'Fraction of each node's area in each region
Public eJacobian As Variant
Public sContact As Variant  'Contact conductance in S/cm2, front to rear, holes to electrons, nNodesX, nNodesY
Public sSigma As Variant
Public sQuadSigma As Variant
Public sFrontIllumination As Variant, sRearIllumination As Variant
Public sFrontHaze As Variant, sRearHaze As Variant
Public sFrontTransmission As Variant, sRearTransmission As Variant
Public sFrontSpecReflect As Variant, sRearSpecReflect As Variant
Public sFrontDiffReflect As Variant, sRearDiffReflect As Variant
Public sJo1 As Variant, sJo2 As Variant, sGsh As Variant        'A/cm2, S/cm2
Public bEnablePointDefect As Boolean, bEnableLineDefect As Boolean, bEnablePlaneDefect As Boolean
Public lineDefectDirection As String, planeDefectDirection As String
Public rPointDefectIo As Double, rLineDefectIo As Double, rPlaneDefectIo As Double
Public rPointDefectN As Double, rLineDefectN As Double, rPlaneDefectN As Double
Public sPhi As Variant          'volts
Public vGeneration As Variant, vRecombination As Variant        'cm-3/s
Public vGamma As Variant, vPsi As Variant
Public vGradGammaX As Variant, vGradGammaY As Variant, vGradGammaZ As Variant
Public vGradPsiX As Variant, vGradPsiY As Variant, vGradPsiZ As Variant
Public fGammaA As Variant, fGammaB As Variant, fGammaC As Variant
Public fPsiA As Variant, fPsiB As Variant, fPsiC As Variant
Public rPsiCo As Double
Public fFrontGammaCsum As Variant, fRearGammaCsum As Variant
Public fFrontPsiCsum As Variant, fRearPsiCsum As Variant
Public sFrontGamma As Variant, sRearGamma As Variant
Public sFrontPsi As Variant, sRearPsi As Variant
Public sFrontGradGamma As Variant, sFrontGradPsi As Variant
Public sRearGradGamma As Variant, sRearGradPsi As Variant
Public sFrontRecomb As Variant, sRearRecomb As Variant
Public sAbsDeltaGradGammaDeltaGamma As Variant
Public sAbsDeltaGradPsiDeltaPsi As Variant
Public savePsi As Variant, saveGamma As Variant, saveRecomb As Variant, savePhi As Variant
Public r2fX As Variant, r2fY As Variant, r2fZ As Variant
Public f2rX As Variant, f2rY As Variant, f2rZ As Variant
Public f2r2X As Variant, f2r2Y As Variant
Public vSinhTermsGammaA As Variant, vSinhTermsGammaB As Variant
Public vSinhTermsPsiA As Variant, vSinhTermsPsiB As Variant
Public vCoshTermsGammaA As Variant, vCoshTermsGammaB As Variant
Public vCoshTermsPsiA As Variant, vCoshTermsPsiB As Variant
Public sinTermsX As Variant, sinTermsY As Variant, sinTermsZ As Variant
Public sPlot As Variant
Public currentPlot As String
Public selectSurface As String, selectQF As String
Public selectJCarrier As String, selectJDirection As String, selectJPortion As String
Public bVolumeScale As Boolean
Public bIntermediatePlot As Boolean
Public bConvergenceFailure As Boolean
Public rTotalGeneration As Double, rTotalRecombination As Double
Public rLambdaAMg As Variant
Public rPowerAMg As Variant
Public rSiAlpha As Variant
Public rSpectrumData As Variant
Public rSpectralLambdas As Variant, rSpectralFront As Variant, rSpectralRear As Variant
Public frontShape As String, rearShape As String
Public rFrontPointX As Double, rFrontPointY As Double, rFrontPointZ As Double, rFrontPointDiameter As Double
Public frontLineDirection As String, frontLineShape As String, rFrontLinePeak As Double, rFrontLineWidth As Double
Public rRearPointX As Double, rRearPointY As Double, rRearPointZ As Double, rRearPointDiameter As Double
Public rearLineDirection As String, rearLineShape As String, rRearLinePeak As Double, rRearLineWidth As Double
Public bSpectrum As Boolean, bMono As Boolean, bFilter As Boolean
Public bGeneration As Boolean, bRecombination As Boolean, bGamma As Boolean, bPsi As Boolean
Public bEnable3D As Boolean, b3D As Boolean, bOpenCircuit As Boolean
Public bSiDiffusivities As Boolean, bEnableTemperatureModel As Boolean, bEnableDopingModel As Boolean
Public bGradGammaPsi As Boolean
Public pType As Boolean, nType As Boolean
Public bContactP As Boolean, bContactN As Boolean
Public bNeedReset As Boolean, bEnableChangeHandlers As Boolean
Public bScan As Boolean, bBatch As Boolean, bStartup As Boolean

'These are spreadsheet cell locations that are read or modified by the code.
'Any re-arrangement of cells on the spreadsheets requires a corresponding change to these constants.
'Most of them are used in the FileHandling module to save and load parameters.
'DASHBOARD
Public Const ivPlot = "Chart 27"
Public Const eqePlot = "Chart 28"
Public Const contourPlot = "Chart 29"
Public Const programNameCell = "Dashboard!K1"
Public Const programVersionCell = "Dashboard!K6"
Public Const dimensionsCell = "Dashboard!B2"
Public Const loadedFileCell = "Dashboard!C1"
Public Const resolutionCell = "Dashboard!D2"
Public Const openCircuitCell = "Dashboard!I1"
Public Const convergenceCell = "Dashboard!D3"
Public Const errorCell = "Dashboard!D4"
Public Const progressCell = "Dashboard!D6"
Public Const iterationCell = "Dashboard!D5"
Public Const voltageCell = "Dashboard!G1"
Public Const ivTypeCell = "Dashboard!B10"
Public Const generationCell = "Dashboard!G2"
Public Const recombinationCell = "Dashboard!G3"
Public Const terminalVoltageCell = "Dashboard!G4"
Public Const terminalCurrentCell = "Dashboard!G5"
Public Const terminalPowerCell = "Dashboard!G6"
Public Const voltageIncrementsCell = "Dashboard!B11"
Public Const darkIVmaxCell = "Dashboard!B12"
Public Const illuminationPowerCell = "Dashboard!B13"
Public Const efficiencyCell = "Dashboard!B14"
Public Const ivWarningCell = "Dashboard!B15"
Public Const iscCell = "Dashboard!D10"
Public Const vocCell = "Dashboard!D11"
Public Const pmaxCell = "Dashboard!D12"
Public Const ffCell = "Dashboard!D13"
Public Const eqeSideCell = "Dashboard!H10"
Public Const minLambdaCell = "Dashboard!H11"
Public Const maxLambdaCell = "Dashboard!H12"
Public Const eqeIncrementsCell = "Dashboard!H13"
Public Const eqeBiasCell = "Dashboard!H14"
Public Const eqeFastCell = "Dashboard!L10"
Public Const eqeTotalCurrentCell = "Dashboard!K12"
Public Const eqeWarningCell = "Dashboard!H15"
Public Const lastPlotViewCell = "Dashboard!S16"
Public Const viewCell = "Dashboard!P9"
Public Const positionCell = "Dashboard!R9"
Public Const unitCell = "Dashboard!P10"
Public Const minCell = "Dashboard!R10"
Public Const maxCell = "Dashboard!T10"
Public Const titleCell = "Dashboard!R8"
Public Const selectSurfaceCell = "Dashboard!P6"
Public Const selectQFCell = "Dashboard!R6"
Public Const selectJCarrierCell = "Dashboard!T4"
Public Const selectJDirectionCell = "Dashboard!T5"
Public Const selectJPortionCell = "Dashboard!T6"
Public Const dashboardChangeCells = viewCell & ", " & positionCell & ", " & selectSurfaceCell _
                & ", " & selectQFCell & ", " & selectJCarrierCell & ", " & selectJDirectionCell _
                & ", " & selectJPortionCell
'DEVICE
Public Const enable3DCell = "Device!M2"
Public Const deviceWidthCell = "Device!B2"
Public Const deviceLengthCell = "Device!E2"
Public Const deviceHeightCell = "Device!H2"
Public Const deviceAreaCell = "Device!K2"
Public Const temperatureCell = "Device!O2"
Public Const deviceDescriptionCell = "Device!R2"
Public Const deviceSeriesCell = "Device!T8"
Public Const deviceDopingCell = "Device!T15"
Public Const enableSiDiffusivitiesCell = "Device!R15"
Public Const deviceDnCell = "Device!T16"
Public Const deviceDpCell = "Device!T17"
Public Const resistivityCell = "Device!T18"
Public Const minorityCell = "Device!T19"
Public Const frontRowLengthCells = "Device!G6:G10"
Public Const frontColumnWidthCells = "Device!B11:F11"
Public Const rearRowLengthCells = "Device!O6:O10"
Public Const rearColumnWidthCells = "Device!J11:N11"
Public Const frontWidthUnitCells = "Device!B12:F12"
Public Const frontLengthUnitCells = "Device!H6:H10"
Public Const rearLengthUnitCells = "Device!P6:P10"
Public Const rearWidthUnitCells = "Device!J12:N12"
Public Const spectralTransCells = "Device!R23:T41"
Public Const spectralLambdasCells = "Device!R23:R41"
Public Const spectralFrontCells = "Device!S23:S41"
Public Const spectralRearCells = "Device!T23:T41"
Public Const frontContactCells = "Device!B6:F10"
Public Const frontSheetRhoCells = "Device!B15:F19"
Public Const frontTransCells = "Device!B22:F26"
Public Const frontHazeCells = "Device!B28:F32"
Public Const frontSpecRefCells = "Device!B34:F38"
Public Const frontDiffRefCells = "Device!B40:F44"
Public Const rearContactCells = "Device!J6:N10"
Public Const rearSheetRhoCells = "Device!J15:N19"
Public Const rearTransCells = "Device!J22:N26"
Public Const rearHazeCells = "Device!J28:N32"
Public Const rearSpecRefCells = "Device!J34:N38"
Public Const rearDiffRefCells = "Device!J40:N44"
Public Const deviceKeyChangeCells = deviceWidthCell & ", " & deviceLengthCell & ", " & deviceHeightCell & ", " _
        & deviceDnCell & ", " & deviceDpCell & ", " _
        & frontSheetRhoCells & ", " & rearSheetRhoCells & ", " & frontContactCells & ", " & rearContactCells
Public Const deviceDimensionCells = frontColumnWidthCells & ", " & frontRowLengthCells & ", " _
        & rearColumnWidthCells & ", " & rearRowLengthCells
Public Const deviceUnitCells = frontWidthUnitCells & ", " & frontLengthUnitCells & ", " _
        & rearWidthUnitCells & ", " & rearLengthUnitCells
Public Const deviceOpticsCells = frontTransCells & ", " & frontHazeCells & ", " & frontSpecRefCells _
        & ", " & frontDiffRefCells & ", " & rearTransCells & ", " & rearHazeCells _
        & ", " & rearSpecRefCells & ", " & rearDiffRefCells
Public Const contactWarningCellP = "Device!H3"
Public Const contactWarningCellN = "Device!H4"
Public Const frontDimensionWarningCell = "Device!B13"
Public Const rearDimensionWarningCell = "Device!J13"
'RECOMBINATION
Public Const intrinsicCell = "Recombination!N2"
Public Const tauNCell = "Recombination!C2"
Public Const tauNUnitsCell = "Recombination!D2"
Public Const tauPCell = "Recombination!C3"
Public Const tauPUnitsCell = "Recombination!D3"
Public Const augerCell = "Recombination!N3"
Public Const lowInjectionLengthCell = "Recombination!H2"
Public Const highInjectionLengthCell = "Recombination!H3"
Public Const recombBulkCell = "Recombination!H1"
Public Const recombFrontJo1Cell = "Recombination!D5"
Public Const recombRearJo1Cell = "Recombination!L5"
Public Const recombFrontJo2Cell = "Recombination!D19"
Public Const recombRearJo2Cell = "Recombination!L19"
Public Const recombDefectCell = "Recombination!H13"
Public Const recombBulkPercentCell = "Recombination!K1"
Public Const recombFrontJo1PercentCell = "Recombination!F5"
Public Const recombRearJo1PercentCell = "Recombination!N5"
Public Const recombFrontJo2PercentCell = "Recombination!F19"
Public Const recombRearJo2PercentCell = "Recombination!N19"
Public Const recombDefectPercentCell = "Recombination!K13"
Public Const frontJo1Cells = "Recombination!B7:F11"
Public Const frontJo2Cells = "Recombination!B21:F25"
Public Const frontGshCells = "Recombination!B27:F31"
Public Const rearJo1Cells = "Recombination!J7:N11"
Public Const rearJo2Cells = "Recombination!J21:N25"
Public Const rearGshCells = "Recombination!J27:N31"
Public Const lineDefectDirectionCell = "Recombination!E16"
Public Const planeDefectDirectionCell = "Recombination!E17"
Public Const defectEnableCells = "Recombination!D15:D17"
Public Const defectPositionCells = "Recombination!F15:H17"
Public Const defectIoCells = "Recombination!J15:J17"
Public Const defectNCells = "Recombination!L15:L17"
Public Const defectDampingCells = "Recombination!N15:N17"
Public Const lifetimeValueCells = tauNCell & ", " & tauPCell & ", " & intrinsicCell & ", " & augerCell
Public Const lifetimeUnitCells = tauNUnitsCell & ", " & tauPUnitsCell
Public Const surfaceRecombinationCells = frontJo1Cells & ", " & frontJo2Cells & ", " & frontGshCells _
                & ", " & rearJo1Cells & ", " & rearJo2Cells & ", " & rearGshCells
Public Const bulkRecombinationValueCells = defectPositionCells & ", " & defectIoCells & ", " & defectNCells
Public Const bulkRecombinationDropdownCells = defectEnableCells _
                & ", " & lineDefectDirectionCell & ", " & planeDefectDirectionCell
'ILLUMINATION
Public Const spectrumCell = "Illumination!B2"
Public Const frontSunsCell = "Illumination!C4"
Public Const rearSunsCell = "Illumination!C5"
Public Const frontPowerCell = "Illumination!C7"
Public Const rearPowerCell = "Illumination!C8"
Public Const lambdaCell = "Illumination!C9"
Public Const opticalFilterCell = "Illumination!C10"
Public Const filterBandgapCell = "Illumination!C11"
Public Const filterEdgeCell = "Illumination!C12"
Public Const filterSubgapCell = "Illumination!C13"
Public Const frontShapeCell = "Illumination!G2"
Public Const frontPointXcenterCell = "Illumination!G5"
Public Const frontPointYcenterCell = "Illumination!G6"
Public Const frontPointZcenterCell = "Illumination!G7"
Public Const frontPointDiameterCell = "Illumination!G8"
Public Const frontLineDirectionCell = "Illumination!G10"
Public Const frontLineShapeCell = "Illumination!G11"
Public Const frontLinePeakCell = "Illumination!G12"
Public Const frontLineWidthCell = "Illumination!G13"
Public Const rearShapeCell = "Illumination!J2"
Public Const rearPointXcenterCell = "Illumination!J5"
Public Const rearPointYcenterCell = "Illumination!J6"
Public Const rearPointZcenterCell = "Illumination!J7"
Public Const rearPointDiameterCell = "Illumination!J8"
Public Const rearLineDirectionCell = "Illumination!J10"
Public Const rearLineShapeCell = "Illumination!J11"
Public Const rearLinePeakCell = "Illumination!J12"
Public Const rearLineWidthCell = "Illumination!J13"
Public Const spectrumDataCells = "Illumination!A17:D76"
Public Const illuminationValueCells = frontSunsCell & ", " & rearSunsCell & ", " & frontPowerCell & ", " & rearPowerCell _
    & ", " & lambdaCell & ", " & filterEdgeCell & ", " & filterSubgapCell & ", " & filterSubgapCell
Public Const illuminationShapeCells = frontPointXcenterCell & ", " & frontPointYcenterCell & ", " & frontPointZcenterCell _
    & ", " & frontPointDiameterCell & ", " & frontLinePeakCell & ", " & frontLineWidthCell & ", " _
    & rearPointXcenterCell & ", " & rearPointYcenterCell & ", " & rearPointZcenterCell _
    & ", " & rearPointDiameterCell & ", " & rearLinePeakCell & ", " & rearLineWidthCell
Public Const illuminationDropdownCells = spectrumCell & ", " & opticalFilterCell & ", " & frontShapeCell _
    & ", " & rearShapeCell & ", " & frontLineDirectionCell & ", " & frontLineShapeCell _
    & ", " & rearLineDirectionCell & ", " & rearLineShapeCell
'DATA
Public Const columnarHeaderCells = "Data!H1:J1"
Public Const tabularUnitsCell = "Data!M2"
Public Const plotNameCell = "Data!J1"
Public Const plotViewCell = "Data!K2"
Public Const plotPositionCell = "Data!K4"
Public Const plotUnitsCell = "Data!K6"
Public Const plotPlaneMinCell = "Data!K8"
Public Const plotPlaneMaxCell = "Data!K10"
Public Const plotVolumeMinCell = "Data!K12"
Public Const plotVolumeMaxCell = "Data!K14"
Public Const plotResolutionCell = "Data!K16"
Public Const plotSurfaceCell = "Data!K18"
Public Const plotCarrierCell = "Data!K20"
Public Const plotDirectionCell = "Data!K22"
Public Const plotPortionCell = "Data!K24"
Public Const tabularAbscissaCells = "Data!N2:BB2"
Public Const tabularOrdinateCells = "Data!M3:M43"
Public Const normalizedDataCells = "Data!N47:BB87"
Public Const columnarDataCells = "Data!H2:J1682"
Public Const tabularDataCells = "Data!N3:BB43"
Public Const normalizedChartCells = "Data!M46:BB87"
Public Const IVrange = "Data!A4:C54"
Public Const eqeRange = "Data!D4:F54"
Public Const uniformityRange = "Data!A58:C98"
'MODELS
Public Const enableTemperatureCell = "Models!A2"
Public Const modelTempNiCells = "Models!A5:B7"
Public Const modelTempOpticsCells = "Models!E4:E5"
Public Const modelTempExponentCell = "Models!I4"
Public Const modelTempSurfaceCells = "Models!N4:N6"
Public Const enableDopingCell = "Models!A11"
Public Const modelDopingBulkCells = "Models!B13:B14"
Public Const modelDopingSurfaceCells = "Models!E13:E14"
Public Const modelJLCell = "Models!B18"
Public Const modelJo1Cell = "Models!B19"
Public Const modelJo2Cell = "Models!B20"
Public Const modelGshCell = "Models!E18"
Public Const modelRsCell = "Models!E19"
Public Const modelTCell = "Models!E20"
Public Const modelRMSCell = "Models!I18"
Public Const modelPmaxCell = "Models!I19"
Public Const modelVocCell = "Models!I20"
Public Const modelGenCell = "Models!C24", modelGenPctCell = "Models!D24"
Public Const modelRecombCell = "Models!C25", modelRecombPctCell = "Models!D25"
Public Const modelTransportCell = "Models!C26", modelTransportPctCell = "Models!D26"
Public Const modelResistanceCell = "Models!C27", modelResistancePctCell = "Models!D27"
Public Const modelTotalCell = "Models!C28", modelTotalPctCell = "Models!D28"
Public Const modelRecombFrontCell = "Models!G25", modelRecombFrontPctCell = "Models!H25"
Public Const modelRecombRearCell = "Models!J25", modelRecombRearPctCell = "Models!K25"
Public Const modelRecombBulkCell = "Models!M25", modelRecombBulkPctCell = "Models!N25"
Public Const modelTransportFrontCell = "Models!G26", modelTransportFrontPctCell = "Models!H26"
Public Const modelTransportRearCell = "Models!J26", modelTransportRearPctCell = "Models!K26"
Public Const modelTransportBulkCell = "Models!M26", modelTransportBulkPctCell = "Models!N26"
Public Const modelResistanceFrontCell = "Models!G27", modelResistanceFrontPctCell = "Models!H27"
Public Const modelResistanceRearCell = "Models!J27", modelResistanceRearPctCell = "Models!K27"
Public Const modelResistanceSeriesCell = "Models!M27", modelResistanceSeriesPctCell = "Models!N27"
Public Const modelTotalFrontCell = "Models!G28", modelTotalFrontPctCell = "Models!H28"
Public Const modelTotalRearCell = "Models!J28", modelTotalRearPctCell = "Models!K28"
Public Const modelTotalBulkCell = "Models!M28", modelTotalBulkPctCell = "Models!N28"
'BATCH
Public Const batchMaxIntervals = 25
Public Const batchMaxOutputValues = 10
Public Const batchRunningCell = "Batch!A2"
Public Const batchDescriptionCell = "Batch!C3"
Public Const batchModeCell = "Batch!A5"
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
Call MapInputParameters     'Fills arrays necessary to deterimine if 3D is required. Sets rInvL2.
'Establish resolution in each axis, then allocate memory for spatial variables.
b3D = Requires3D        'Assess whether 3D is needed due to nonuniformity in the y-axis (length)
If Range(resolutionCell) = "Auto" Then      'Must set X and Y before calling Z
    nHarmonicsX = MinHarmonics(xAxis)
    nHarmonicsY = MinHarmonics(yAxis)
    nHarmonicsZ = MinHarmonics(zAxis)
Else
    nHarmonicsX = Range(resolutionCell)
    nHarmonicsY = Range(resolutionCell)
    nHarmonicsZ = Range(resolutionCell)
End If
If Not b3D Then nHarmonicsY = 1     'While no harmonics are needed in 2D, at least one is required for interpolation routines.
nNodesX = nHarmonicsX + 1
nNodesY = nHarmonicsY + 1
nNodesZ = nHarmonicsZ + 1
ReDim nElementIDx(front To rear, 1 To nNodesX) As Integer
ReDim nElementIDy(front To rear, 1 To nNodesY) As Integer
ReDim elementBoundariesX(front To rear, 1 To nElements) As Double
ReDim elementBoundariesY(front To rear, 1 To nElements) As Double
ReDim eRegionLowerX(front To rear, 1 To nElementNodes) As Integer
ReDim eRegionUpperX(front To rear, 1 To nElementNodes) As Integer
ReDim eRegionLowerY(front To rear, 1 To nElementNodes) As Integer
ReDim eRegionUpperY(front To rear, 1 To nElementNodes) As Integer
ReDim eElementLowerX(front To rear, 1 To nElements) As Integer
ReDim eElementUpperX(front To rear, 1 To nElements) As Integer
ReDim eElementLowerY(front To rear, 1 To nElements) As Integer
ReDim eElementUpperY(front To rear, 1 To nElements) As Integer
ReDim eNodeAllocationX(front To rear, 1 To nNodesX, 1 To nRegions) As Double
ReDim eNodeAllocationY(front To rear, 1 To nNodesY, 1 To nRegions) As Double
ReDim sContact(front To rear, holes To electrons, 1 To nNodesX, 1 To nNodesY) As Double
ReDim sSigma(front To rear, 1 To nNodesX, 1 To nNodesY) As Double
ReDim sQuadSigma(front To rear, holes To electrons, 1 To nNodesX, 1 To nNodesY, 1 To 4) As Double
ReDim sFrontHaze(1 To nNodesX, 1 To nNodesY) As Double, sRearHaze(1 To nNodesX, 1 To nNodesY) As Double
ReDim sFrontTransmission(1 To nNodesX, 1 To nNodesY) As Double, sRearTransmission(1 To nNodesX, 1 To nNodesY) As Double
ReDim sFrontSpecReflect(1 To nNodesX, 1 To nNodesY) As Double, sRearSpecReflect(1 To nNodesX, 1 To nNodesY) As Double
ReDim sFrontDiffReflect(1 To nNodesX, 1 To nNodesY) As Double, sRearDiffReflect(1 To nNodesX, 1 To nNodesY) As Double
ReDim sJo1(front To rear, 1 To nNodesX, 1 To nNodesY) As Double
ReDim sJo2(front To rear, 1 To nNodesX, 1 To nNodesY) As Double
ReDim sGsh(front To rear, 1 To nNodesX, 1 To nNodesY) As Double
ReDim sAbsDeltaGradGammaDeltaGamma(1 To nNodesX, 1 To nNodesY) As Double
ReDim sAbsDeltaGradPsiDeltaPsi(1 To nNodesX, 1 To nNodesY) As Double
ReDim vSinhTermsGammaA(1 To nHarmonicsX + 1, 1 To nHarmonicsY + 1, 1 To nNodesZ) As Double
ReDim vSinhTermsGammaB(1 To nHarmonicsX + 1, 1 To nHarmonicsY + 1, 1 To nNodesZ) As Double
ReDim vSinhTermsPsiA(1 To nHarmonicsX + 1, 1 To nHarmonicsY + 1, 1 To nNodesZ) As Double
ReDim vSinhTermsPsiB(1 To nHarmonicsX + 1, 1 To nHarmonicsY + 1, 1 To nNodesZ) As Double
ReDim vCoshTermsGammaA(1 To nHarmonicsX + 1, 1 To nHarmonicsY + 1, 1 To nNodesZ) As Double
ReDim vCoshTermsGammaB(1 To nHarmonicsX + 1, 1 To nHarmonicsY + 1, 1 To nNodesZ) As Double
ReDim vCoshTermsPsiA(1 To nHarmonicsX + 1, 1 To nHarmonicsY + 1, 1 To nNodesZ) As Double
ReDim vCoshTermsPsiB(1 To nHarmonicsX + 1, 1 To nHarmonicsY + 1, 1 To nNodesZ) As Double
ReDim sinTermsX(1 To nNodesX, 1 To nHarmonicsX + 1) As Double
ReDim sinTermsY(1 To nNodesY, 1 To nHarmonicsY + 1) As Double
ReDim sinTermsZ(1 To nNodesZ, 1 To nHarmonicsZ + 1) As Double
ReDim sPlot(1 To nMaxNodes, 1 To nMaxNodes) As Double
ReDim eJacobian(1 To 4 * nElementNodes * nElementNodes, 1 To 4 * nElementNodes * nElementNodes) As Double
ReDim sPhi(front To rear, holes To electrons, 1 To nNodesX, 1 To nNodesY) As Double
ReDim fFrontGammaCsum(1 To nHarmonicsX + 1, 1 To nHarmonicsY + 1) As Double
ReDim fRearGammaCsum(1 To nHarmonicsX + 1, 1 To nHarmonicsY + 1) As Double
ReDim fFrontPsiCsum(1 To nHarmonicsX + 1, 1 To nHarmonicsY + 1) As Double
ReDim fRearPsiCsum(1 To nHarmonicsX + 1, 1 To nHarmonicsY + 1) As Double
ReDim sFrontGamma(1 To nNodesX, 1 To nNodesY) As Double
ReDim sRearGamma(1 To nNodesX, 1 To nNodesY) As Double
ReDim sFrontPsi(1 To nNodesX, 1 To nNodesY) As Double
ReDim sRearPsi(1 To nNodesX, 1 To nNodesY) As Double
ReDim sFrontGradGamma(1 To nNodesX, 1 To nNodesY) As Double
ReDim sFrontGradPsi(1 To nNodesX, 1 To nNodesY) As Double
ReDim sRearGradGamma(1 To nNodesX, 1 To nNodesY) As Double
ReDim sRearGradPsi(1 To nNodesX, 1 To nNodesY) As Double
ReDim sFrontRecomb(1 To nNodesX, 1 To nNodesY) As Double
ReDim sRearRecomb(1 To nNodesX, 1 To nNodesY) As Double
ReDim savePsi(1 To nNodesX, 1 To nNodesY) As Double
ReDim saveGamma(1 To nNodesX, 1 To nNodesY) As Double
ReDim saveRecomb(1 To nNodesX, 1 To nNodesY) As Double
ReDim savePhi(front To rear, holes To electrons, 1 To nNodesX, 1 To nNodesY) As Double
ReDim vPointRecombination(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
ReDim vLineRecombination(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
ReDim vPlaneRecombination(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
ReDim vRecombination(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
ReDim vGamma(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
ReDim vPsi(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
ReDim vGradGammaX(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
ReDim vGradGammaY(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
ReDim vGradGammaZ(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
ReDim vGradPsiX(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
ReDim vGradPsiY(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
ReDim vGradPsiZ(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
ReDim fGammaA(1 To nHarmonicsX + 1, 1 To nHarmonicsY + 1) As Double
ReDim fGammaB(1 To nHarmonicsX + 1, 1 To nHarmonicsY + 1) As Double
ReDim fGammaC(1 To nHarmonicsX + 1, 1 To nHarmonicsY + 1, 1 To nHarmonicsZ + 1) As Double
ReDim fPsiA(1 To nHarmonicsX + 1, 1 To nHarmonicsY + 1) As Double
ReDim fPsiB(1 To nHarmonicsX + 1, 1 To nHarmonicsY + 1) As Double
ReDim fPsiC(1 To nHarmonicsX + 1, 1 To nHarmonicsY + 1, 1 To nHarmonicsZ + 1) As Double
rLambdaAMg = Array(310#, 320#, 330#, 340#, 350#, 370#, 390#, 410#, 430#, 450#, 470#, 490#, 510#, 530#, 550#, 590#, 630#, 670#, 710#, 730#, 750#, 770#, 790#, 810#, 830#, 860#, 900#, 920#, 940#, 960#, 990#, 1070#, 1120#, 1140#, 1180#, 1220#, 1280#, 1360#, 1460#, 1540#, 1580#, 1610#, 1640#, 1740#, 1860#, 1960#, 2000#, 2060#, 2150#, 2270#, 2450#, 2540#, 2980#, 3060#, 3160#, 3240#, 3340#, 3580#, 4000#)
rSiAlpha = Array(1440000#, 1280000#, 1170000#, 1090000#, 1040000#, 697000#, 150000#, 67400#, 39200#, 25500#, 17200#, 12700#, 9700#, 7850#, 6390#, 4490#, 3270#, 2390#, 1780#, 1540#, 1300#, 1100#, 927#, 707#, 646#, 480#, 306#, 200#, 183#, 134#, 79.2, 8#, 2#, 1#, 0.065, 0.0082, 0.00012, 0.000001, 0.000001, 0.000001, 0.000001, 0.000001, 0.000001, 0.000001, 0.000001, 0.000001, 0.000001, 0.000001, 0.000001, 0.000001, 0.000001, 0.000001, 0.000001, 0.000001, 0.000001, 0.000001, 0.000001, 0.000001, 0.000001)
rPowerAMg = Array(0.72, 2.098, 4.137, 4.54, 7.8, 13.24, 14.554, 23.596, 23.964, 29.922, 31.471, 30.971, 30.34, 30.786, 46.055, 59.071, 57.249, 54.424, 37.318, 22.895, 23.689, 19.435, 22.38, 19.899, 23.96, 33.649, 23.915, 13.4, 5.945, 12.296, 39.5, 45.457, 5.609, 6.446, 17.074, 22.693, 27.044, 5.102, 7.075, 15.49, 8.718, 7.067, 14.372, 16.759, 0.149, 1.452, 2.875, 6.055, 8.367, 9.837, 3.682, 0.058, 0.478, 0.276, 0.611, 0.307, 0.644, 1.5, 1.584)
rTotalGeneration = 0
rTotalRecombination = 0
rPsiCo = 0
rInvL2 = 1
currentPlot = "Layout"
bGeneration = False: bRecombination = False
bVolumeScale = False: bIntermediatePlot = False
bConvergenceFailure = False
bScan = False: bBatch = False
bGamma = False: bPsi = False: bGradGammaPsi = False
End Sub

Sub ResetAll()
'Initializes all global variables set by programming.
'Calls routines that map input values from spreadsheet, clears some spreadsheet cells, then plots Layout.
'Establishes node count in each axis and maps surface parameters onto these nodes.
'ResetAll, LoadFile, and ValidatePosition are the only user routines that set/reset the change handlers directly.
bStartup = False        'Once reset here, the program is no longer in the startup mode

Application.Calculation = xlCalculationManual
bEnableChangeHandlers = False
Range(progressCell).Value = "Reset": DoEvents
Range(batchRunningCell).Value = ""
Call InitializeAll
Call OnDeviceChange     'Implement any interrupted modifications. Calls ResetElectrical
Call OnIlluminationChange   'Implement any interrupted modifications. Calls ResetGeneration
bNeedReset = False  'Must come after above two calls, which set this to True
'Reset terminal characteristics here, rather than in ResetElectrical so that they don't get wiped so frequently
Range(terminalVoltageCell) = ""
Range(terminalCurrentCell) = ""
Range(terminalPowerCell) = ""
Call ClearFreeEnergyAnalysis
If MapSurfaceParameters Then
    Worksheets("Dashboard").ChartObjects(contourPlot).Chart.SetSourceData Source:=Sheets("Data").Range(normalizedChartCells)
    Worksheets("Dashboard").ChartObjects(contourPlot).Chart.ChartGroups(1).Has3DShading = False
    Call PlotLayout
    Range(progressCell).Value = "" 'Clear progress field to indicate Reset is successfully finished
Else
    Range(progressCell).Value = "Error"
    bNeedReset = True
End If
Application.Calculation = xlCalculationAutomatic
bEnableChangeHandlers = True
End Sub

Sub ResetGeneration()
'This routine reinitializes the photogeneration in preparation for a new optical solution.
Call MapInputParameters
ReDim vGeneration(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
ReDim sFrontIllumination(1 To nNodesX, 1 To nNodesY) As Double
ReDim sRearIllumination(1 To nNodesX, 1 To nNodesY) As Double
rTotalGeneration = 0
Range(generationCell).Value = 0
bGeneration = False
End Sub

Sub ResetElectrical()
'This routine reinitializes all electrical values in preparation for a new electrical solution.
Call MapInputParameters
ReDim eJacobian(1 To 4 * nElementNodes * nElementNodes, 1 To 4 * nElementNodes * nElementNodes) As Double
ReDim sPhi(front To rear, holes To electrons, 1 To nNodesX, 1 To nNodesY) As Double
ReDim fFrontGammaCsum(1 To nHarmonicsX + 1, 1 To nHarmonicsY + 1) As Double
ReDim fRearGammaCsum(1 To nHarmonicsX + 1, 1 To nHarmonicsY + 1) As Double
ReDim fFrontPsiCsum(1 To nHarmonicsX + 1, 1 To nHarmonicsY + 1) As Double
ReDim fRearPsiCsum(1 To nHarmonicsX + 1, 1 To nHarmonicsY + 1) As Double
ReDim sFrontGamma(1 To nNodesX, 1 To nNodesY) As Double
ReDim sRearGamma(1 To nNodesX, 1 To nNodesY) As Double
ReDim sFrontPsi(1 To nNodesX, 1 To nNodesY) As Double
ReDim sRearPsi(1 To nNodesX, 1 To nNodesY) As Double
ReDim sFrontGradGamma(1 To nNodesX, 1 To nNodesY) As Double
ReDim sFrontGradPsi(1 To nNodesX, 1 To nNodesY) As Double
ReDim sRearGradGamma(1 To nNodesX, 1 To nNodesY) As Double
ReDim sRearGradPsi(1 To nNodesX, 1 To nNodesY) As Double
ReDim sFrontRecomb(1 To nNodesX, 1 To nNodesY) As Double
ReDim sRearRecomb(1 To nNodesX, 1 To nNodesY) As Double
ReDim savePsi(1 To nNodesX, 1 To nNodesY) As Double
ReDim saveGamma(1 To nNodesX, 1 To nNodesY) As Double
ReDim saveRecomb(1 To nNodesX, 1 To nNodesY) As Double
ReDim savePhi(front To rear, holes To electrons, 1 To nNodesX, 1 To nNodesY) As Double
ReDim vPointRecombination(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
ReDim vLineRecombination(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
ReDim vPlaneRecombination(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
ReDim vRecombination(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
ReDim vGamma(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
ReDim vPsi(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
ReDim vGradGammaX(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
ReDim vGradGammaY(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
ReDim vGradGammaZ(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
ReDim vGradPsiX(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
ReDim vGradPsiY(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
ReDim vGradPsiZ(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
ReDim fGammaA(1 To nHarmonicsX + 1, 1 To nHarmonicsY + 1) As Double
ReDim fGammaB(1 To nHarmonicsX + 1, 1 To nHarmonicsY + 1) As Double
ReDim fGammaC(1 To nHarmonicsX + 1, 1 To nHarmonicsY + 1, 1 To nHarmonicsZ + 1) As Double
ReDim fPsiA(1 To nHarmonicsX + 1, 1 To nHarmonicsY + 1) As Double
ReDim fPsiB(1 To nHarmonicsX + 1, 1 To nHarmonicsY + 1) As Double
ReDim fPsiC(1 To nHarmonicsX + 1, 1 To nHarmonicsY + 1, 1 To nHarmonicsZ + 1) As Double
rTotalRecombination = 0
rPsiCo = 0
bGamma = False: bPsi = False: bGradGammaPsi = False
bRecombination = False

If Not (Range(voltageCell) >= 0 And Range(voltageCell) <= 2) Then Range(voltageCell).Value = 0
Range(resistivityCell).Value = rVt / (q * (rEqP * rDp + rEqN * rDn))
Range(minorityCell).Value = rMinEq
Range(lowInjectionLengthCell).Value = 10000 * Sqr(Abs(rDmin * ((rEqP + rNi) * rTauN + (rEqN + rNi) * rTauP) / (rEqP + rEqN))) 'um
Range(highInjectionLengthCell).Value = 10000 * Sqr(Abs(rDamb * (rTauN + rTauP)))                                               'um
Range(errorCell).Value = ""
Range(iterationCell).Value = ""
Range(recombinationCell).Value = 0
Range(recombBulkCell).Value = 0
Range(recombFrontJo1Cell).Value = 0
Range(recombRearJo1Cell).Value = 0
Range(recombFrontJo2Cell).Value = 0
Range(recombRearJo2Cell).Value = 0
Range(recombDefectCell).Value = 0
Range(recombBulkPercentCell).Value = 0
Range(recombFrontJo1PercentCell).Value = 0
Range(recombRearJo1PercentCell).Value = 0
Range(recombDefectPercentCell).Value = 0
Range(recombFrontJo2PercentCell).Value = 0
Range(recombRearJo2PercentCell).Value = 0
End Sub

Sub ClearFreeEnergyAnalysis()
Range(modelGenCell).Value = ""
Range(modelRecombCell).Value = ""
Range(modelTransportCell).Value = ""
Range(modelResistanceCell).Value = ""
Range(modelTotalCell).Value = ""
Range(modelRecombFrontCell).Value = ""
Range(modelRecombRearCell).Value = ""
Range(modelRecombBulkCell).Value = ""
Range(modelTransportFrontCell).Value = ""
Range(modelTransportRearCell).Value = ""
Range(modelTransportBulkCell).Value = ""
Range(modelResistanceFrontCell).Value = ""
Range(modelResistanceRearCell).Value = ""
Range(modelResistanceSeriesCell).Value = ""
Range(modelTotalFrontCell).Value = ""
Range(modelTotalRearCell).Value = ""
Range(modelTotalBulkCell).Value = ""
Range(modelGenPctCell).Value = ""
Range(modelRecombPctCell).Value = ""
Range(modelTransportPctCell).Value = ""
Range(modelResistancePctCell).Value = ""
Range(modelTotalPctCell).Value = ""
Range(modelRecombFrontPctCell).Value = ""
Range(modelRecombRearPctCell).Value = ""
Range(modelRecombBulkPctCell).Value = ""
Range(modelTransportFrontPctCell).Value = ""
Range(modelTransportRearPctCell).Value = ""
Range(modelTransportBulkPctCell).Value = ""
Range(modelResistanceFrontPctCell).Value = ""
Range(modelResistanceRearPctCell).Value = ""
Range(modelResistanceSeriesPctCell).Value = ""
Range(modelTotalFrontPctCell).Value = ""
Range(modelTotalRearPctCell).Value = ""
Range(modelTotalBulkPctCell).Value = ""
End Sub

