Attribute VB_Name = "Calc2D"
Option Explicit

Sub CalculateA2D()
'This routine takes surface Gamma and Psi to obtain Fourier coefficients
'of Gamma and Psi at the front surface (A).
'Note that the sums of Fourier C terms (bulk effects) must have been calculated previously.
Dim m As Integer
Dim fFrontGamma As Variant
Dim fFrontPsi As Variant
ReDim fFrontGamma(1 To nHarmonicsX + 1, 1 To 1) As Double
ReDim fFrontPsi(1 To nHarmonicsX + 1, 1 To 1) As Double
fFrontGamma = Application.MMult(r2fX, sFrontGamma)
fFrontPsi = Application.MMult(r2fX, sFrontPsi)
'Now calculate A components from surface boundary conditions and store globally
ReDim fGammaA(1 To nHarmonicsX + 1, 1 To 1) As Double
ReDim fPsiA(1 To nHarmonicsX + 1, 1 To 1) As Double
For m = 0 To nHarmonicsX
    fGammaA(m + 1, 1) = fFrontGamma(m + 1, 1) + fFrontGammaCsum(m + 1, 1)
    fPsiA(m + 1, 1) = fFrontPsi(m + 1, 1) + fFrontPsiCsum(m + 1, 1)
Next m
End Sub

Sub CalculateB2D()
'This routine takes surface Gamma and Psi to obtain Fourier coefficients
'of Gamma and Psi at the rear surface (B).
'Note that the sums of Fourier C terms (bulk effects) must have been calculated previously
Dim m As Integer
Dim fRearGamma As Variant
Dim fRearPsi As Variant
ReDim fRearGamma(1 To nHarmonicsX + 1, 1 To 1) As Double
ReDim fRearPsi(1 To nHarmonicsX + 1, 1 To 1) As Double
fRearPsi = Application.MMult(r2fX, sRearPsi)
fRearGamma = Application.MMult(r2fX, sRearGamma)
'Now calculate B components from surface boundary conditions and store globally
ReDim fGammaB(1 To nHarmonicsX + 1, 1 To 1) As Double
ReDim fPsiB(1 To nHarmonicsX + 1, 1 To 1) As Double
For m = 0 To nHarmonicsX
    fGammaB(m + 1, 1) = fRearGamma(m + 1, 1) + fRearGammaCsum(m + 1, 1)
    fPsiB(m + 1, 1) = fRearPsi(m + 1, 1) + fRearPsiCsum(m + 1, 1)
Next m
End Sub

Sub CalculateC2D()
'This routine takes values of vGeneration and VRecombination to calculate Fourier components
'of the C's for Gamma, Psi and the summed C terms for Gamma and Psi at front and rear surfaces (used for A & B)
Dim m As Integer, P As Integer
Dim kmp2 As Double
Dim i As Integer, k As Integer
Dim rPsiFactor As Double, rGammaFactor As Double
Dim localRG As Double, rGamma As Double, rGradGammaGradPsi As Double
Dim sQgamma As Variant, sQpsi As Variant
Dim fQgamma As Variant, fQpsi As Variant
Dim fQgamma2 As Variant, fQpsi2 As Variant
ReDim sQgamma(1 To nNodesX, 1 To nNodesZ) As Double
ReDim sQpsi(1 To nNodesX, 1 To nNodesZ) As Double
ReDim fQgamma(1 To nHarmonicsX + 1, 1 To 1, 1 To nHarmonicsZ + 1) As Double
ReDim fQpsi(1 To nHarmonicsX + 1, 1 To 1, 1 To nHarmonicsZ + 1) As Double
ReDim fQgamma2(1 To nHarmonicsX + 1, 1 To nHarmonicsZ + 1) As Double
ReDim fQpsi2(1 To nHarmonicsX + 1, 1 To nHarmonicsZ + 1) As Double

'Obtain fourier components of Q's (requires vGeneration, vRecombination, vGamma, and spatial derivaties of gamma and psi)
'The LLI treatment is exact within the assumption of quasi-neutrality in the limit of infinite harmonics
rPsiFactor = rVt * (rDn - rDp) / (rDn * rDp)
rGammaFactor = (rEqP - rEqN) / (2 * rVt)

For i = 1 To nNodesX: For k = 1 To nNodesZ
    localRG = vRecombination(i, 1, k) - vGeneration(i, 1, k)
    rGamma = vGamma(i, 1, k)
    rGradGammaGradPsi = vGradGammaX(i, 1, k) * vGradPsiX(i, 1, k) _
                      + vGradGammaZ(i, 1, k) * vGradPsiZ(i, 1, k)
    sQpsi(i, k) = (localRG * rPsiFactor - 2 * rGradGammaGradPsi) / (rEqN + rEqP + 2 * rGamma)
    sQgamma(i, k) = localRG / rDamb - rGammaFactor * sQpsi(i, k) - rGamma * rInvL2
Next k: Next i

fQgamma2 = Application.Transpose(Application.MMult(r2fZ, Application.Transpose(Application.MMult(r2fX, sQgamma))))
fQpsi2 = Application.Transpose(Application.MMult(r2fZ, Application.Transpose(Application.MMult(r2fX, sQpsi))))
For m = 0 To nHarmonicsX: For P = 0 To nHarmonicsZ
    fQgamma(m + 1, 1, P + 1) = fQgamma2(m + 1, P + 1)
    fQpsi(m + 1, 1, P + 1) = fQpsi2(m + 1, P + 1)
Next P: Next m
'Calculate C coefficients from Q components
rPsiCo = fQpsi(1, 1, 1)
For m = 0 To nHarmonicsX: For P = 0 To nHarmonicsZ
    kmp2 = (m * Pi / deviceWidth) ^ 2 + (P * Pi / deviceHeight) ^ 2
    fGammaC(m + 1, 1, P + 1) = fQgamma(m + 1, 1, P + 1) / (kmp2 + rInvL2)
    If kmp2 > 0 Then
        fPsiC(m + 1, 1, P + 1) = fQpsi(m + 1, 1, P + 1) / kmp2
    Else
        fPsiC(m + 1, 1, P + 1) = 0
    End If
Next P: Next m
'Calculate Csum terms used to calculate A and B terms
ReDim fFrontGammaCsum(1 To nHarmonicsX + 1, 1 To 1) As Double
ReDim fRearGammaCsum(1 To nHarmonicsX + 1, 1 To 1) As Double
ReDim fFrontPsiCsum(1 To nHarmonicsX + 1, 1 To 1) As Double
ReDim fRearPsiCsum(1 To nHarmonicsX + 1, 1 To 1) As Double
For m = 0 To nHarmonicsX
    fFrontGammaCsum(m + 1, 1) = 0: fRearGammaCsum(m + 1, 1) = 0
    fFrontPsiCsum(m + 1, 1) = 0: fRearPsiCsum(m + 1, 1) = 0
    For P = 0 To nHarmonicsZ
        fFrontGammaCsum(m + 1, 1) = fFrontGammaCsum(m + 1, 1) + fGammaC(m + 1, 1, P + 1)
        fRearGammaCsum(m + 1, 1) = fRearGammaCsum(m + 1, 1) + fGammaC(m + 1, 1, P + 1) * Cos(Pi * P)
        fFrontPsiCsum(m + 1, 1) = fFrontPsiCsum(m + 1, 1) + fPsiC(m + 1, 1, P + 1)
        fRearPsiCsum(m + 1, 1) = fRearPsiCsum(m + 1, 1) + fPsiC(m + 1, 1, P + 1) * Cos(Pi * P)
    Next P
Next m
End Sub

Sub CalculateRealGamma2D()
'Assumes uniformity in the y-axis to reduce computation time.
Dim i As Integer, j As Integer, k As Integer
Dim m As Integer
Dim fGammaAB As Variant, vGammaC As Variant, sGamma As Variant, saveGamma As Variant
Dim newGamma As Double
'Calculate the real-space C components of vGamma
ReDim vGammaC(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
ReDim fGammaAB(1 To nHarmonicsX + 1, 1 To nNodesZ) As Double
ReDim sGamma(1 To nNodesX, 1 To nNodesZ) As Double
ReDim saveGamma(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
saveGamma = vGamma
vGammaC = MTrans3Dback(MMult3D(f2rZ, MTrans3Drp(MMult3D(f2rY, MTrans3Drc(MMult3D(f2rX, fGammaC))))))
'Calculate vGamma (global) in real space
For m = 0 To nHarmonicsX: For k = 1 To nNodesZ
    fGammaAB(m + 1, k) = fGammaA(m + 1, 1) * vSinhTermsGammaA(m + 1, 1, k) _
                       + fGammaB(m + 1, 1) * vSinhTermsGammaB(m + 1, 1, k)
Next k: Next m
sGamma = Application.MMult(f2rX, fGammaAB)
For i = 1 To nNodesX: For j = 1 To nNodesY: For k = 1 To nNodesZ
    newGamma = sGamma(i, k) - vGammaC(i, j, k)
    If newGamma <= 0 Then vGamma(i, j, k) = saveGamma(i, j, k) / 2 Else vGamma(i, j, k) = newGamma
Next k: Next j: Next i
bGamma = True
End Sub

Sub CalculateRealPsi2D()
'If 2D, assumes uniformity in the y-axis to reduce computation time.
Dim i As Integer, j As Integer, k As Integer
Dim m As Integer
Dim z As Double
Dim fPsiAB As Variant, vPsiC As Variant, sPsi As Variant
'Calculate the real-space C components of vPsi
ReDim vPsiC(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
ReDim fPsiAB(1 To nHarmonicsX + 1, 1 To nNodesZ) As Double
ReDim sPsi(1 To nNodesX, 1 To nNodesZ) As Double
vPsiC = MTrans3Dback(MMult3D(f2rZ, MTrans3Drp(MMult3D(f2rY, MTrans3Drc(MMult3D(f2rX, fPsiC))))))
'Calculate vPsi (global) in real space
For m = 0 To nHarmonicsX: For k = 1 To nNodesZ
    fPsiAB(m + 1, k) = fPsiA(m + 1, 1) * vSinhTermsPsiA(m + 1, 1, k) _
                     + fPsiB(m + 1, 1) * vSinhTermsPsiB(m + 1, 1, k)
Next k: Next m
sPsi = Application.MMult(f2rX, fPsiAB)
For i = 1 To nNodesX: For j = 1 To nNodesY: For k = 1 To nNodesZ
    z = (k - 1) * deviceHeight / (nNodesZ - 1)
    vPsi(i, j, k) = sPsi(i, k) - vPsiC(i, j, k) - 0.5 * rPsiCo * z * (deviceHeight - z)
Next k: Next j: Next i
bPsi = True
End Sub

Sub CalculateGradGammaPsi2D()
'This routine uses the fourier coefficients Amn, Bmn, Cmnp, Co to find the
'x-, y-, and z-components of the gradient of Gamma and Psi at all nodes i,j,k
'Assumes uniformity in the y-axis to reduce computation time.
Dim i As Integer, j As Integer, k As Integer
Dim m As Integer, P As Integer
Dim z As Double
Dim fGradAB As Variant, fGammaC2 As Variant, fPsiC2 As Variant
Dim sGradAB As Variant, sGradC2 As Variant
ReDim fGradAB(1 To nHarmonicsX + 1, 1 To nNodesZ) As Double
ReDim fGammaC2(1 To nHarmonicsX + 1, 1 To nNodesZ) As Double
ReDim fPsiC2(1 To nHarmonicsX + 1, 1 To nNodesZ) As Double
ReDim sGradAB(1 To nNodesX, 1 To nNodesZ) As Double
ReDim sGradC2(1 To nNodesX, 1 To nNodesZ) As Double

'GAMMA
For m = 0 To nHarmonicsX: For P = 0 To nHarmonicsZ
    fGammaC2(m + 1, P + 1) = fGammaC(m + 1, 1, P + 1)
Next P: Next m
'X axis
For m = 0 To nHarmonicsX: For k = 1 To nNodesZ
    fGradAB(m + 1, k) = -fGammaA(m + 1, 1) * vSinhTermsGammaA(m + 1, 1, k) _
                       - fGammaB(m + 1, 1) * vSinhTermsGammaB(m + 1, 1, k)
Next k: Next m
sGradAB = Application.MMult(sinTermsX, fGradAB)
sGradC2 = Application.MMult(sinTermsX, Application.Transpose(Application.MMult(f2rZ, Application.Transpose(fGammaC2))))
For i = 1 To nNodesX: For j = 1 To nNodesY: For k = 1 To nNodesZ
    vGradGammaX(i, j, k) = sGradAB(i, k) + sGradC2(i, k)
Next k: Next j: Next i
'Z axis
For m = 0 To nHarmonicsX: For k = 1 To nNodesZ
    fGradAB(m + 1, k) = -fGammaA(m + 1, 1) * vCoshTermsGammaA(m + 1, 1, k) _
                       + fGammaB(m + 1, 1) * vCoshTermsGammaB(m + 1, 1, k)
Next k: Next m
sGradAB = Application.MMult(f2rX, fGradAB)
sGradC2 = Application.MMult(f2rX, Application.Transpose(Application.MMult(sinTermsZ, Application.Transpose(fGammaC2))))
For i = 1 To nNodesX: For j = 1 To nNodesY: For k = 1 To nNodesZ
    vGradGammaZ(i, j, k) = sGradAB(i, k) + sGradC2(i, k)
Next k: Next j: Next i

'PSI
For m = 0 To nHarmonicsX: For P = 0 To nHarmonicsZ
    fPsiC2(m + 1, P + 1) = fPsiC(m + 1, 1, P + 1)
Next P: Next m
'X axis
For m = 0 To nHarmonicsX: For k = 1 To nNodesZ
    fGradAB(m + 1, k) = -fPsiA(m + 1, 1) * vSinhTermsPsiA(m + 1, 1, k) _
                       - fPsiB(m + 1, 1) * vSinhTermsPsiB(m + 1, 1, k)
Next k: Next m
sGradAB = Application.MMult(sinTermsX, fGradAB)
sGradC2 = Application.MMult(sinTermsX, Application.Transpose(Application.MMult(f2rZ, Application.Transpose(fPsiC2))))
For i = 1 To nNodesX: For j = 1 To nNodesY: For k = 1 To nNodesZ
    vGradPsiX(i, j, k) = sGradAB(i, k) + sGradC2(i, k)
Next k: Next j: Next i
'Z axis
For m = 0 To nHarmonicsX: For k = 1 To nNodesZ
    fGradAB(m + 1, k) = -fPsiA(m + 1, 1) * vCoshTermsPsiA(m + 1, 1, k) _
                       + fPsiB(m + 1, 1) * vCoshTermsPsiB(m + 1, 1, k)
Next k: Next m
sGradAB = Application.MMult(f2rX, fGradAB)
sGradC2 = Application.MMult(f2rX, Application.Transpose(Application.MMult(sinTermsZ, Application.Transpose(fPsiC2))))
For k = 1 To nNodesZ 'Requires special treatment due to rPsiCo term
    z = (k - 1) * deviceHeight / (nNodesZ - 1)
    For i = 1 To nNodesX: For j = 1 To nNodesY
        vGradPsiZ(i, j, k) = sGradAB(i, k) + sGradC2(i, k) + rPsiCo * (z - 0.5 * deviceHeight)
    Next j: Next i
Next k
bGradGammaPsi = True
End Sub

Sub CalculateSurfaceGradGammaPsi2D()
'This routine uses the fourier coefficients Amn, Bmn, Cmnp, Co to find the
'z-component of the gradient of Gamma and Psi at each surface
'Note that C terms are all zero at the surfaces, except for rPsiCo
Dim i As Integer, j As Integer, k As Integer
Dim m As Integer
Dim fGradAB As Variant    'fourier-transform coefficients
Dim sGradAB As Variant    'real-space values
ReDim fGradAB(1 To nHarmonicsX + 1, 1 To 1) As Double
ReDim sGradAB(1 To nNodesX, 1 To 1) As Double

'***FRONT***
'First calculate Grad Gamma
For m = 0 To nHarmonicsX
    fGradAB(m + 1, 1) = -fGammaA(m + 1, 1) * vCoshTermsGammaA(m + 1, 1, 1) _
                       + fGammaB(m + 1, 1) * vCoshTermsGammaB(m + 1, 1, 1)
Next m
sGradAB = Application.MMult(f2rX, fGradAB)
For i = 1 To nNodesX: For j = 1 To nNodesY
    sFrontGradGamma(i, j) = sGradAB(i, 1)
Next j: Next i
'Now calculate Grad Psi
For m = 0 To nHarmonicsX
    fGradAB(m + 1, 1) = -fPsiA(m + 1, 1) * vCoshTermsPsiA(m + 1, 1, 1) _
                       + fPsiB(m + 1, 1) * vCoshTermsPsiB(m + 1, 1, 1)
Next m
sGradAB = Application.MMult(f2rX, fGradAB)
For i = 1 To nNodesX: For j = 1 To nNodesY
    sFrontGradPsi(i, j) = sGradAB(i, 1) - 0.5 * rPsiCo * deviceHeight
Next j: Next i
'***REAR***
'First calculate Grad Gamma
For m = 0 To nHarmonicsX
    fGradAB(m + 1, 1) = -fGammaA(m + 1, 1) * vCoshTermsGammaA(m + 1, 1, nNodesZ) _
                       + fGammaB(m + 1, 1) * vCoshTermsGammaB(m + 1, 1, nNodesZ)
Next m
sGradAB = Application.MMult(f2rX, fGradAB)
For i = 1 To nNodesX: For j = 1 To nNodesY
    sRearGradGamma(i, j) = sGradAB(i, 1)
Next j: Next i
'Now calculate Grad Psi
For m = 0 To nHarmonicsX
    fGradAB(m + 1, 1) = -fPsiA(m + 1, 1) * vCoshTermsPsiA(m + 1, 1, nNodesZ) _
                       + fPsiB(m + 1, 1) * vCoshTermsPsiB(m + 1, 1, nNodesZ)
Next m
sGradAB = Application.MMult(f2rX, fGradAB)
For i = 1 To nNodesX: For j = 1 To nNodesY
    sRearGradPsi(i, j) = sGradAB(i, 1) + 0.5 * rPsiCo * deviceHeight
Next j: Next i
End Sub

