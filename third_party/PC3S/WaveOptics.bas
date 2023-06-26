Attribute VB_Name = "WaveOptics"
Option Explicit
Type Complex
    Real As Double
    Imaginary As Double
End Type

Sub ARC(ByVal nPolarization As Integer, ByVal rAngle As Double, ByVal rLambda0 As Double, _
                 ByRef rReflected As Double, ByRef rTransmitted As Double)
'Returns the complex values for reflected and transmitted fields for an absorptive triple-layer anti-relfection coating.
'The physical parameters of the ARC (index, absorption, and thickness) are global variables
'rAngle is the angle of incidence with respect to the normal vector of the ARC stack
'rLambda0 is the free-space wavelength (may not match the wavelength in encapsulant layer 1)
'Caution: Note that the order of layers in the spreadsheet is substrate to encapsulant, whereas this calculation runs encapsulant to substrate
Dim beta As Double      'Free-space wavenumber
Dim Eph As Double        'Photon energy in eV
Dim phi2 As Complex, phi3 As Complex, phi4 As Complex       'Layer's single-pass phase shift
Dim reflect12 As Complex, reflect23 As Complex, reflect34 As Complex, reflect45 As Complex
Dim transmit12 As Complex, transmit23 As Complex, transmit34  As Complex, transmit45 As Complex, transmit15 As Complex
Dim reflect1 As Complex, reflect2 As Complex, reflect3 As Complex, reflect4 As Complex
Dim transmit2 As Complex, transmit3 As Complex, transmit4 As Complex
Dim n1 As Complex, n2 As Complex, n3 As Complex, n4 As Complex, n5 As Complex
Dim sin1 As Complex, sin2 As Complex, sin3 As Complex, sin4 As Complex, sin5 As Complex
Dim cos1 As Complex, cos2 As Complex, cos3 As Complex, cos4 As Complex, cos5 As Complex
Dim d2 As Double, d3 As Double, d4 As Double   'Layer thicknesses, in nm
Dim n As Double    'Layer index
Dim k As Double     'Layer extinction coefficient
Dim kBand As Double, kFCA As Double
Dim rAlphaSi As Double      'nm-1
Dim rIndexSi As Double      'dimensionless
Dim lambdaEG As Double  'nm
Dim Eg As Double    'eV
Const gammaBand As Double = 2, gammaFCA As Double = 3
Eph = hc / rLambda0
beta = 2 * pi / rLambda0
'Set up layer optical parameters
n1.Real = rEncapsulationIndex
n1.Imaginary = 0
'Layer 2 (ARC layer adjacent to encapsulant)
d2 = rCoatingData(1, 3)
n = rCoatingData(2, 3) + rCoatingData(3, 3) * (Eph - 2)
If n < 1 Then n = 1
Eg = rCoatingData(4, 3): If Eg <= 0 Then Eg = Eph
lambdaEG = hc / Eg
kBand = rCoatingData(5, 3)
kFCA = rCoatingData(6, 3)
k = kFCA * (rLambda0 / lambdaEG) ^ gammaFCA
If Eph > Eg Then k = k + kBand * (Eph / Eg - 1) ^ gammaBand
n2.Real = n
n2.Imaginary = k
'Layer 3 (ARC layer in middle of sandwhich)
d3 = rCoatingData(1, 2)
n = rCoatingData(2, 2) + rCoatingData(3, 2) * (Eph - 2)
If n < 1 Then n = 1
Eg = rCoatingData(4, 2): If Eg <= 0 Then Eg = Eph
lambdaEG = hc / Eg
kBand = rCoatingData(5, 2)
kFCA = rCoatingData(6, 2)
k = kFCA * (rLambda0 / lambdaEG) ^ gammaFCA
If Eph > Eg Then k = k + kBand * (Eph / Eg - 1) ^ gammaBand
n3.Real = n
n3.Imaginary = k
'Layer 4 (ARC layer adjacent to silicon)
d4 = rCoatingData(1, 1)
n = rCoatingData(2, 1) + rCoatingData(3, 1) * (Eph - 2)
If n < 1 Then n = 1
Eg = rCoatingData(4, 1): If Eg <= 0 Then Eg = Eph
lambdaEG = hc / Eg
kBand = rCoatingData(5, 1)
kFCA = rCoatingData(6, 1)
k = kFCA * (rLambda0 / lambdaEG) ^ gammaFCA
If Eph > Eg Then k = k + kBand * (Eph / Eg - 1) ^ gammaBand
n4.Real = n
n4.Imaginary = k
'Silicon substrate
rAlphaSi = InterpolateColumns(rLambda0, 1, rSpectrumData, 3, rSpectrumData, False, False) * 0.0000001
rIndexSi = InterpolateColumns(rLambda0, 1, rSpectrumData, 4, rSpectrumData, False, False)
n5.Real = rIndexSi
n5.Imaginary = rAlphaSi * rLambda0 / (4 * pi)

sin1.Real = Sin(rAngle): sin1.Imaginary = 0
sin2 = CDiv(CMult(n1, sin1), n2)
sin3 = CDiv(CMult(n2, sin2), n3)
sin4 = CDiv(CMult(n3, sin3), n4)
sin5 = CDiv(CMult(n4, sin4), n5)
cos1 = CRoot(CSub(one, CMult(sin1, sin1)))
cos2 = CRoot(CSub(one, CMult(sin2, sin2)))
cos3 = CRoot(CSub(one, CMult(sin3, sin3)))
cos4 = CRoot(CSub(one, CMult(sin4, sin4)))
cos5 = CRoot(CSub(one, CMult(sin5, sin5)))

reflect12 = CReflectance(nPolarization, n1, cos1, n2, cos2)
reflect23 = CReflectance(nPolarization, n2, cos2, n3, cos3)
reflect34 = CReflectance(nPolarization, n3, cos3, n4, cos4)
reflect45 = CReflectance(nPolarization, n4, cos4, n5, cos5)

transmit12 = CTransmittance(nPolarization, n1, cos1, n2, cos2)
transmit23 = CTransmittance(nPolarization, n2, cos2, n3, cos3)
transmit34 = CTransmittance(nPolarization, n3, cos3, n4, cos4)
transmit45 = CTransmittance(nPolarization, n4, cos4, n5, cos5)

phi2 = CScale(CMult(n2, cos2), beta * d2)
phi3 = CScale(CMult(n3, cos3), beta * d3)
phi4 = CScale(CMult(n4, cos4), beta * d4)

'Effective reflectance at bottom of each layer, used to find effective reflectance of next layer up the stack
reflect4 = reflect45
reflect3 = CLayerReflectance(reflect34, reflect4, phi4)
reflect2 = CLayerReflectance(reflect23, reflect3, phi3)
reflect1 = CLayerReflectance(reflect12, reflect2, phi2)
rReflected = CMagnitude(reflect1) ^ 2

'Transmittance across each layer (omitting final exit transmittance)
transmit2 = CLayerTransmittance(transmit12, reflect12, reflect2, phi2)
transmit3 = CLayerTransmittance(transmit23, reflect23, reflect3, phi3)
transmit4 = CLayerTransmittance(transmit34, reflect34, reflect4, phi4)
transmit15 = CMult(CMult(CMult(transmit2, transmit3), transmit4), transmit45)

rTransmitted = (n5.Real * cos5.Real) / (n1.Real * cos1.Real) * CMagnitude(transmit15) ^ 2

End Sub

'*** COMPLEX REFLECTANCE AND TRANSMISSION ***

Function CReflectance(ByVal nPolarization As Integer, n1 As Complex, cos1 As Complex, n2 As Complex, cos2 As Complex) As Complex
Select Case nPolarization
    Case TE
        CReflectance = CDiv(CSub(CMult(n1, cos1), CMult(n2, cos2)), CAdd(CMult(n1, cos1), CMult(n2, cos2)))
    Case TM
        CReflectance = CDiv(CSub(CMult(n2, cos1), CMult(n1, cos2)), CAdd(CMult(n2, cos1), CMult(n1, cos2)))
End Select
End Function

Function CTransmittance(ByVal nPolarization As Integer, n1 As Complex, cos1 As Complex, n2 As Complex, cos2 As Complex) As Complex
Select Case nPolarization
    Case TE
        CTransmittance = CScale(CDiv(CMult(n1, cos1), CAdd(CMult(n1, cos1), CMult(n2, cos2))), 2)
    Case TM
        CTransmittance = CScale(CDiv(CMult(n1, cos1), CAdd(CMult(n2, cos1), CMult(n1, cos2))), 2)
End Select
End Function

Function CLayerReflectance(topR As Complex, botR As Complex, phi As Complex) As Complex
CLayerReflectance = CDiv(CAdd(topR, CMult(botR, CExp(CMult(j, CScale(phi, 2))))), CAdd(one, CMult(topR, CMult(botR, CExp(CMult(j, CScale(phi, 2)))))))
End Function

Function CLayerTransmittance(topT As Complex, topR As Complex, botR As Complex, phi As Complex) As Complex
CLayerTransmittance = CDiv(CMult(topT, CExp(CMult(j, phi))), CAdd(one, CMult(topR, CMult(botR, CExp(CMult(j, CScale(phi, 2)))))))
End Function

'*** COMPLEX MATH FUNCTIONS ***

Function zero() As Complex
zero.Real = 0
zero.Imaginary = 0
End Function

Function one() As Complex
one.Real = 1
one.Imaginary = 0
End Function

Function j() As Complex
j.Real = 0
j.Imaginary = 1
End Function

Function CNeg(c1 As Complex) As Complex
CNeg.Real = -c1.Real
CNeg.Imaginary = -c1.Imaginary
End Function

Function CConjugate(c1 As Complex) As Complex
CConjugate.Real = c1.Real
CConjugate.Imaginary = -c1.Imaginary
End Function

Function CScale(c1 As Complex, ByVal c2 As Double) As Complex
CScale.Real = c1.Real * c2
CScale.Imaginary = c1.Imaginary * c2
End Function

Function CSquared(c1 As Complex) As Double
CSquared = c1.Real ^ 2 + c1.Imaginary ^ 2
End Function

Function CMagnitude(c1 As Complex) As Double
CMagnitude = Sqr(c1.Real ^ 2 + c1.Imaginary ^ 2)
End Function

Function CInvert(c1 As Complex) As Complex
'Note returns zero if c1 equals 0
Dim magnitude As Double, phase As Double
magnitude = Sqr(c1.Real ^ 2 + c1.Imaginary ^ 2)
If magnitude > 0 Then
    phase = WorksheetFunction.Atan2(c1.Real, c1.Imaginary)
    magnitude = 1 / magnitude
    phase = -phase
End If
CInvert.Real = magnitude * Cos(phase)
CInvert.Imaginary = magnitude * Sin(phase)
End Function

Function CAdd(c1 As Complex, c2 As Complex) As Complex
CAdd.Real = c1.Real + c2.Real
CAdd.Imaginary = c1.Imaginary + c2.Imaginary
End Function

Function CSub(c1 As Complex, c2 As Complex) As Complex
CSub.Real = c1.Real - c2.Real
CSub.Imaginary = c1.Imaginary - c2.Imaginary
End Function

Function CMult(c1 As Complex, c2 As Complex) As Complex
CMult.Real = c1.Real * c2.Real - c1.Imaginary * c2.Imaginary
CMult.Imaginary = c1.Real * c2.Imaginary + c1.Imaginary * c2.Real
End Function

Function CDiv(c1 As Complex, c2 As Complex) As Complex
CDiv = CMult(c1, CInvert(c2))
End Function

Function CExp(c1 As Complex) As Complex
CExp.Real = ExpPower(c1.Real) * Cos(c1.Imaginary)
CExp.Imaginary = ExpPower(c1.Real) * Sin(c1.Imaginary)
End Function

Function CRoot(c1 As Complex) As Complex
Dim magnitude As Double, phase As Double
magnitude = Sqr(c1.Real ^ 2 + c1.Imaginary ^ 2)
If magnitude > 0 Then phase = WorksheetFunction.Atan2(c1.Real, c1.Imaginary)
magnitude = Sqr(magnitude)
phase = phase / 2
CRoot.Real = magnitude * Cos(phase)
CRoot.Imaginary = magnitude * Sin(phase)
End Function
