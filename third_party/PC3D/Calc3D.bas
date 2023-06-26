Attribute VB_Name = "Calc3D"
Option Explicit

Sub CalculateA3D()
'This routine takes surface Gamma and Psi to obtain Fourier coefficients
'of Gamma and Psi at the the front surface (A).
'Note that the sums of Fourier C terms (bulk effects) must have been calculated previously.
Dim m As Integer, n As Integer
Dim fFrontGamma As Variant
Dim fFrontPsi As Variant
ReDim fFrontGamma(1 To nHarmonicsX + 1, 1 To nHarmonicsY + 1) As Double
ReDim fFrontPsi(1 To nHarmonicsX + 1, 1 To nHarmonicsY + 1) As Double
fFrontPsi = Application.Transpose(Application.MMult(r2fY, Application.Transpose(Application.MMult(r2fX, sFrontPsi))))
fFrontGamma = Application.Transpose(Application.MMult(r2fY, Application.Transpose(Application.MMult(r2fX, sFrontGamma))))
'Now calculate A components from surface boundary conditions and store globally
ReDim fGammaA(1 To nHarmonicsX + 1, 1 To nHarmonicsY + 1) As Double
ReDim fPsiA(1 To nHarmonicsX + 1, 1 To nHarmonicsY + 1) As Double
For m = 0 To nHarmonicsX: For n = 0 To nHarmonicsY
    fGammaA(m + 1, n + 1) = fFrontGamma(m + 1, n + 1) + fFrontGammaCsum(m + 1, n + 1)
    fPsiA(m + 1, n + 1) = fFrontPsi(m + 1, n + 1) + fFrontPsiCsum(m + 1, n + 1)
Next n: Next m
End Sub

Sub CalculateB3D()
'This routine takes surface Gamma and Psi to obtain Fourier coefficients
'of Gamma and Psi at the rear surface (B).
'Note that the sums of Fourier C terms (bulk effects) must have been calculated previously
Dim m As Integer, n As Integer
Dim fRearGamma As Variant
Dim fRearPsi As Variant
ReDim fRearGamma(1 To nHarmonicsX + 1, 1 To nHarmonicsY + 1) As Double
ReDim fRearPsi(1 To nHarmonicsX + 1, 1 To nHarmonicsY + 1) As Double
fRearPsi = Application.Transpose(Application.MMult(r2fY, Application.Transpose(Application.MMult(r2fX, sRearPsi))))
fRearGamma = Application.Transpose(Application.MMult(r2fY, Application.Transpose(Application.MMult(r2fX, sRearGamma))))
'Now calculate B components from surface boundary conditions and store globally
ReDim fGammaB(1 To nHarmonicsX + 1, 1 To nHarmonicsY + 1) As Double
ReDim fPsiB(1 To nHarmonicsX + 1, 1 To nHarmonicsY + 1) As Double
For m = 0 To nHarmonicsX: For n = 0 To nHarmonicsY
    fGammaB(m + 1, n + 1) = fRearGamma(m + 1, n + 1) + fRearGammaCsum(m + 1, n + 1)
    fPsiB(m + 1, n + 1) = fRearPsi(m + 1, n + 1) + fRearPsiCsum(m + 1, n + 1)
Next n: Next m
End Sub

Sub CalculateC3D()
'This routine takes values of vGeneration and VRecombination to calculate Fourier components
'of the C's for Gamma, Psi and the summed C terms for Gamma and Psi at front and rear surfaces (used for A & B)
Dim m As Integer, n As Integer, P As Integer
Dim kmnp2 As Double
Dim i As Integer, j As Integer, k As Integer
Dim rPsiFactor As Double, rGammaFactor As Double
Dim localRG As Double, rGamma As Double, rGradGammaGradPsi As Double
Dim vQgamma As Variant, vQpsi As Variant
Dim sQgamma As Variant, sQpsi As Variant
Dim fQgamma As Variant, fQpsi As Variant
Dim fQgamma2 As Variant, fQpsi2 As Variant
ReDim vQgamma(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
ReDim vQpsi(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
ReDim sQgamma(1 To nNodesX, 1 To nNodesY) As Double
ReDim sQpsi(1 To nNodesX, 1 To nNodesY) As Double
ReDim fQgamma(1 To nHarmonicsX + 1, 1 To nHarmonicsY + 1, 1 To nHarmonicsZ + 1) As Double
ReDim fQpsi(1 To nHarmonicsX + 1, 1 To nHarmonicsY + 1, 1 To nHarmonicsZ + 1) As Double
ReDim fQgamma2(1 To nHarmonicsX + 1, 1 To nHarmonicsY + 1) As Double
ReDim fQpsi2(1 To nHarmonicsX + 1, 1 To nHarmonicsY + 1) As Double

'Obtain fourier components of Q's (requires vGeneration, vRecombination, vGamma, and gradients of Gamma and Psi)
'This is exact within the assumption of quasi-neutrality in the limit of infinite harmonics
rPsiFactor = rVt * (rDn - rDp) / (rDn * rDp)
rGammaFactor = (rEqP - rEqN) / (2 * rVt)

For i = 1 To nNodesX: For j = 1 To nNodesY: For k = 1 To nNodesZ
    localRG = vRecombination(i, j, k) - vGeneration(i, j, k)
    rGamma = vGamma(i, j, k)
    rGradGammaGradPsi = vGradGammaX(i, j, k) * vGradPsiX(i, j, k) _
                      + vGradGammaY(i, j, k) * vGradPsiY(i, j, k) _
                      + vGradGammaZ(i, j, k) * vGradPsiZ(i, j, k)
    vQpsi(i, j, k) = (localRG * rPsiFactor - 2 * rGradGammaGradPsi) / (rEqN + rEqP + 2 * rGamma)
    vQgamma(i, j, k) = localRG / rDamb - rGammaFactor * vQpsi(i, j, k) - rGamma * rInvL2
Next k: Next j: Next i
fQgamma = MTrans3Dback(MMult3D(r2fZ, MTrans3Drp(MMult3D(r2fY, MTrans3Drc(MMult3D(r2fX, vQgamma))))))
fQpsi = MTrans3Dback(MMult3D(r2fZ, MTrans3Drp(MMult3D(r2fY, MTrans3Drc(MMult3D(r2fX, vQpsi))))))

'Calculate C coefficients from Q components
rPsiCo = fQpsi(1, 1, 1)
For m = 0 To nHarmonicsX: For n = 0 To nHarmonicsY: For P = 0 To nHarmonicsZ
    kmnp2 = (m * Pi / deviceWidth) ^ 2 + (n * Pi / deviceLength) ^ 2 + (P * Pi / deviceHeight) ^ 2
    fGammaC(m + 1, n + 1, P + 1) = fQgamma(m + 1, n + 1, P + 1) / (kmnp2 + rInvL2)
    If kmnp2 > 0 Then
        fPsiC(m + 1, n + 1, P + 1) = fQpsi(m + 1, n + 1, P + 1) / kmnp2
    Else
        fPsiC(m + 1, n + 1, P + 1) = 0
    End If
Next P: Next n: Next m
'Calculate Csum terms used to calculate A and B terms
ReDim fFrontGammaCsum(1 To nHarmonicsX + 1, 1 To nHarmonicsY + 1) As Double
ReDim fRearGammaCsum(1 To nHarmonicsX + 1, 1 To nHarmonicsY + 1) As Double
ReDim fFrontPsiCsum(1 To nHarmonicsX + 1, 1 To nHarmonicsY + 1) As Double
ReDim fRearPsiCsum(1 To nHarmonicsX + 1, 1 To nHarmonicsY + 1) As Double
For m = 0 To nHarmonicsX: For n = 0 To nHarmonicsY
    fFrontGammaCsum(m + 1, n + 1) = 0: fRearGammaCsum(m + 1, n + 1) = 0
    fFrontPsiCsum(m + 1, n + 1) = 0: fRearPsiCsum(m + 1, n + 1) = 0
    For P = 0 To nHarmonicsZ
        fFrontGammaCsum(m + 1, n + 1) = fFrontGammaCsum(m + 1, n + 1) + fGammaC(m + 1, n + 1, P + 1)
        fRearGammaCsum(m + 1, n + 1) = fRearGammaCsum(m + 1, n + 1) + fGammaC(m + 1, n + 1, P + 1) * Cos(Pi * P)
        fFrontPsiCsum(m + 1, n + 1) = fFrontPsiCsum(m + 1, n + 1) + fPsiC(m + 1, n + 1, P + 1)
        fRearPsiCsum(m + 1, n + 1) = fRearPsiCsum(m + 1, n + 1) + fPsiC(m + 1, n + 1, P + 1) * Cos(Pi * P)
    Next P
Next n: Next m
End Sub

Sub CalculateRealGamma3D()
Dim i As Integer, j As Integer, k As Integer
Dim m As Integer, n As Integer
Dim fGammaAB As Variant, vGammaC As Variant, saveGamma As Variant
Dim newGamma As Double
'Calculate the real-space C components of vGamma
ReDim vGammaC(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
ReDim saveGamma(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
saveGamma = vGamma
vGammaC = MTrans3Dback(MMult3D(f2rZ, MTrans3Drp(MMult3D(f2rY, MTrans3Drc(MMult3D(f2rX, fGammaC))))))
'Calculate vGamma (global) in real space
ReDim fGammaAB(1 To nHarmonicsX + 1, 1 To nHarmonicsY + 1, 1 To nNodesZ) As Double
For m = 0 To nHarmonicsX: For n = 0 To nHarmonicsY: For k = 1 To nNodesZ
    fGammaAB(m + 1, n + 1, k) = fGammaA(m + 1, n + 1) * vSinhTermsGammaA(m + 1, n + 1, k) _
                              + fGammaB(m + 1, n + 1) * vSinhTermsGammaB(m + 1, n + 1, k)
Next k: Next n: Next m
vGamma = MTrans3Drc(MMult3D(f2rY, MTrans3Drc(MMult3D(f2rX, fGammaAB))))
For i = 1 To nNodesX: For j = 1 To nNodesY: For k = 1 To nNodesZ
    newGamma = vGamma(i, j, k) - vGammaC(i, j, k)
    If newGamma <= 0 Then vGamma(i, j, k) = saveGamma(i, j, k) / 2 Else vGamma(i, j, k) = newGamma
Next k: Next j: Next i
bGamma = True
End Sub

Sub CalculateRealPsi3D()
Dim i As Integer, j As Integer, k As Integer
Dim m As Integer, n As Integer
Dim z As Double
Dim fPsiAB As Variant, vPsiC As Variant
'Calculate the real-space C components of vPsi
ReDim vPsiC(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
vPsiC = MTrans3Dback(MMult3D(f2rZ, MTrans3Drp(MMult3D(f2rY, MTrans3Drc(MMult3D(f2rX, fPsiC))))))
'Calculate vPsi (global) in real space
ReDim fPsiAB(1 To nHarmonicsX + 1, 1 To nHarmonicsY + 1, 1 To nNodesZ) As Double
For m = 0 To nHarmonicsX: For n = 0 To nHarmonicsY: For k = 1 To nNodesZ
    fPsiAB(m + 1, n + 1, k) = fPsiA(m + 1, n + 1) * vSinhTermsPsiA(m + 1, n + 1, k) _
                            + fPsiB(m + 1, n + 1) * vSinhTermsPsiB(m + 1, n + 1, k)
Next k: Next n: Next m
vPsi = MTrans3Drc(MMult3D(f2rY, MTrans3Drc(MMult3D(f2rX, fPsiAB))))
For i = 1 To nNodesX: For j = 1 To nNodesY: For k = 1 To nNodesZ
    z = (k - 1) * deviceHeight / (nNodesZ - 1)
    vPsi(i, j, k) = vPsi(i, j, k) - vPsiC(i, j, k) - 0.5 * rPsiCo * z * (deviceHeight - z)
Next k: Next j: Next i
bPsi = True
End Sub

Sub CalculateGradGammaPsi3D()
'This routine uses the fourier coefficients Amn, Bmn, Cmnp, Co to find the
'x-, y-, and z-components of the gradient of Gamma and Psi at all nodes i,j,k
Dim i As Integer, j As Integer, k As Integer
Dim m As Integer, n As Integer
Dim z As Double
Dim fGradAB As Variant
Dim vGradAB As Variant, vGradC As Variant
ReDim fGradAB(1 To nHarmonicsX + 1, 1 To nHarmonicsY + 1, 1 To nNodesZ) As Double
ReDim vGradAB(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
ReDim vGradC(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double

'GAMMA
For m = 0 To nHarmonicsX: For n = 0 To nHarmonicsY: For k = 1 To nNodesZ
    fGradAB(m + 1, n + 1, k) = -fGammaA(m + 1, n + 1) * vSinhTermsGammaA(m + 1, n + 1, k) _
                             - fGammaB(m + 1, n + 1) * vSinhTermsGammaB(m + 1, n + 1, k)
Next k: Next n: Next m
'X axis
vGradAB = MTrans3Drc(MMult3D(f2rY, MTrans3Drc(MMult3D(sinTermsX, fGradAB))))
vGradC = MTrans3Drc(MMult3D(f2rY, MTrans3Drc(MMult3D(sinTermsX, MTrans3Drp(MMult3D(f2rZ, MTrans3Drp(fGammaC)))))))
vGradGammaX = MAdd3D(vGradAB, vGradC)
'Y axis
vGradAB = MTrans3Drc(MMult3D(sinTermsY, MTrans3Drc(MMult3D(f2rX, fGradAB))))
vGradC = MTrans3Drc(MMult3D(sinTermsY, MTrans3Drc(MMult3D(f2rX, MTrans3Drp(MMult3D(f2rZ, MTrans3Drp(fGammaC)))))))
vGradGammaY = MAdd3D(vGradAB, vGradC)
'Z axis
For m = 0 To nHarmonicsX: For n = 0 To nHarmonicsY: For k = 1 To nNodesZ
    fGradAB(m + 1, n + 1, k) = -fGammaA(m + 1, n + 1) * vCoshTermsGammaA(m + 1, n + 1, k) _
                             + fGammaB(m + 1, n + 1) * vCoshTermsGammaB(m + 1, n + 1, k)
Next k: Next n: Next m
vGradAB = MTrans3Drc(MMult3D(f2rY, MTrans3Drc(MMult3D(f2rX, fGradAB))))
vGradC = MTrans3Drc(MMult3D(f2rY, MTrans3Drc(MMult3D(f2rX, MTrans3Drp(MMult3D(sinTermsZ, MTrans3Drp(fGammaC)))))))
vGradGammaZ = MAdd3D(vGradAB, vGradC)
'PSI
For m = 0 To nHarmonicsX: For n = 0 To nHarmonicsY: For k = 1 To nNodesZ
    fGradAB(m + 1, n + 1, k) = -fPsiA(m + 1, n + 1) * vSinhTermsPsiA(m + 1, n + 1, k) _
                             - fPsiB(m + 1, n + 1) * vSinhTermsPsiB(m + 1, n + 1, k)
Next k: Next n: Next m
'X axis
vGradAB = MTrans3Drc(MMult3D(f2rY, MTrans3Drc(MMult3D(sinTermsX, fGradAB))))
vGradC = MTrans3Drc(MMult3D(f2rY, MTrans3Drc(MMult3D(sinTermsX, MTrans3Drp(MMult3D(f2rZ, MTrans3Drp(fPsiC)))))))
vGradPsiX = MAdd3D(vGradAB, vGradC)
'Y axis
vGradAB = MTrans3Drc(MMult3D(sinTermsY, MTrans3Drc(MMult3D(f2rX, fGradAB))))
vGradC = MTrans3Drc(MMult3D(sinTermsY, MTrans3Drc(MMult3D(f2rX, MTrans3Drp(MMult3D(f2rZ, MTrans3Drp(fPsiC)))))))
vGradPsiY = MAdd3D(vGradAB, vGradC)
'Z axis
For m = 0 To nHarmonicsX: For n = 0 To nHarmonicsY: For k = 1 To nNodesZ
    fGradAB(m + 1, n + 1, k) = -fPsiA(m + 1, n + 1) * vCoshTermsPsiA(m + 1, n + 1, k) _
                             + fPsiB(m + 1, n + 1) * vCoshTermsPsiB(m + 1, n + 1, k)
Next k: Next n: Next m
vGradAB = MTrans3Drc(MMult3D(f2rY, MTrans3Drc(MMult3D(f2rX, fGradAB))))
vGradC = MTrans3Drc(MMult3D(f2rY, MTrans3Drc(MMult3D(f2rX, MTrans3Drp(MMult3D(sinTermsZ, MTrans3Drp(fPsiC)))))))
For k = 1 To nNodesZ 'Requires special treatment due to rPsiCo term
    z = (k - 1) * deviceHeight / (nNodesZ - 1)
    For i = 1 To nNodesX: For j = 1 To nNodesY
        vGradPsiZ(i, j, k) = vGradAB(i, j, k) + vGradC(i, j, k) + rPsiCo * (z - 0.5 * deviceHeight)
    Next j: Next i
Next k
bGradGammaPsi = True
End Sub

Sub CalculateSurfaceGradGammaPsi3D()
'This routine uses the fourier coefficients Amn, Bmn, Cmnp, Co to find the
'z-component of the gradient of Gamma and Psi at each surface
'Note that C terms are all zero at the surfaces, except for rPsiCo
Dim i As Integer, j As Integer, k As Integer
Dim m As Integer, n As Integer
Dim fGradAB As Variant    'fourier-transform coefficients
Dim sGradAB As Variant    'real-space values
ReDim fGradAB(1 To nHarmonicsX + 1, 1 To nHarmonicsY + 1) As Double
ReDim sGradAB(1 To nNodesX, 1 To nNodesY) As Double

'***FRONT***
'First calculate Grad Gamma
For m = 0 To nHarmonicsX: For n = 0 To nHarmonicsY
    fGradAB(m + 1, n + 1) = -fGammaA(m + 1, n + 1) * vCoshTermsGammaA(m + 1, n + 1, 1) _
                          + fGammaB(m + 1, n + 1) * vCoshTermsGammaB(m + 1, n + 1, 1)
Next n: Next m
sFrontGradGamma = Application.MMult(f2rX, Application.Transpose(Application.MMult(f2rY, Application.Transpose(fGradAB))))
'Now calculate Grad Psi
For m = 0 To nHarmonicsX: For n = 0 To nHarmonicsY
    fGradAB(m + 1, n + 1) = -fPsiA(m + 1, n + 1) * vCoshTermsPsiA(m + 1, n + 1, 1) _
                          + fPsiB(m + 1, n + 1) * vCoshTermsPsiB(m + 1, n + 1, 1)
Next n: Next m
sGradAB = Application.MMult(f2rX, Application.Transpose(Application.MMult(f2rY, Application.Transpose(fGradAB))))
For i = 1 To nNodesX: For j = 1 To nNodesY
    sFrontGradPsi(i, j) = sGradAB(i, j) - 0.5 * rPsiCo * deviceHeight
Next j: Next i
'***REAR***
'First calculate Grad Gamma
For m = 0 To nHarmonicsX: For n = 0 To nHarmonicsY
    fGradAB(m + 1, n + 1) = -fGammaA(m + 1, n + 1) * vCoshTermsGammaA(m + 1, n + 1, nNodesZ) _
                          + fGammaB(m + 1, n + 1) * vCoshTermsGammaB(m + 1, n + 1, nNodesZ)
Next n: Next m
sRearGradGamma = Application.MMult(f2rX, Application.Transpose(Application.MMult(f2rY, Application.Transpose(fGradAB))))
'Now calculate Grad Psi
For m = 0 To nHarmonicsX: For n = 0 To nHarmonicsY
    fGradAB(m + 1, n + 1) = -fPsiA(m + 1, n + 1) * vCoshTermsPsiA(m + 1, n + 1, nNodesZ) _
                          + fPsiB(m + 1, n + 1) * vCoshTermsPsiB(m + 1, n + 1, nNodesZ)
Next n: Next m
sGradAB = Application.MMult(f2rX, Application.Transpose(Application.MMult(f2rY, Application.Transpose(fGradAB))))
For i = 1 To nNodesX: For j = 1 To nNodesY
    sRearGradPsi(i, j) = sGradAB(i, j) + 0.5 * rPsiCo * deviceHeight
Next j: Next i
End Sub

