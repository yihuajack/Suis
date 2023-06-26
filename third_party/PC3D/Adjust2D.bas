Attribute VB_Name = "Adjust2D"
Option Explicit

Sub AdjustSurfaces2D()
'This routine evaluates the effect of perturbing each of these element node's variables on the flux balance
'for holes and electrons within each region. adjusts the values of gamma and psi at the surfaces in an
'attempt to eliminate flux imbalance at the surface nodes.
Const nElementNodesX = nElementNodes
Dim i As Integer, iElementNode As Integer
Dim iSurface As Integer, iVariable As Integer, iElement As Integer
Dim rDeltaGamma As Double, rDeltaPsi As Double
Dim F0 As Variant, f As Variant
Dim negativeDeltas As Variant
Dim eDelta As Variant
ReDim eJacobian(1 To 4 * nElementNodesX, 1 To 4 * nElementNodesX) As Double
ReDim F0(1 To 4 * nElementNodesX, 1 To 1) As Double  'Errors at element nodes before perturbation
ReDim f(1 To 4 * nElementNodesX, 1 To 1) As Double   'Errors at element nodes after perturbation
ReDim negativeDeltas(1 To 4 * nElementNodesX, 1) As Double
ReDim eDelta(front To rear, gamma To psi, 1 To nElementNodesX) As Double

Call CalculateSurfaceGradGammaPsi2D
Call ElementFluxErrors2D(F0)    'Set reference (unperturbed) element flux error matrix

'Front Gamma
rDeltaGamma = FunctionDeltaGamma
For iElementNode = 1 To nElementNodesX
    Call saveSurface(front)    'Saves surface Gamma, Psi, and Recomb
    Call IncrementVariable2D(front, gamma, iElementNode, rDeltaGamma) 'Perturbs Gamma and Recomb
    Call CalculateA
    Call CalculateSurfaceGradGammaPsi2D
    Call ElementFluxErrors2D(f)
    Call LoadJacobianCol2D(front, gamma, iElementNode, rDeltaGamma, f, F0)
    Call RestoreSurface(front)  'Restores surface Gamma, Psi, and Recomb
Next iElementNode
Call CalculateA

'Front Psi
rDeltaPsi = FunctionDeltaPsi
For iElementNode = 1 To nElementNodesX
    Call saveSurface(front)     'Saves surface Gamma, Psi, and Recomb
    Call IncrementVariable2D(front, psi, iElementNode, rDeltaPsi) 'Perturbs Psi only
    Call CalculateA
    Call CalculateSurfaceGradGammaPsi2D
    Call ElementFluxErrors2D(f)
    Call LoadJacobianCol2D(front, psi, iElementNode, rDeltaPsi, f, F0)
    Call RestoreSurface(front) 'Restores surface Gamma, Psi, and Recomb
Next iElementNode
Call CalculateA     'Restore GammaA, PsiA

'Rear Gamma
rDeltaGamma = FunctionDeltaGamma
For iElementNode = 1 To nElementNodesX
    Call saveSurface(rear)    'Saves surface Gamma,Psi, and Recomb
    Call IncrementVariable2D(rear, gamma, iElementNode, rDeltaGamma)   'Perturbs Gamma and Recomb
    Call CalculateB
    Call CalculateSurfaceGradGammaPsi2D
    Call ElementFluxErrors2D(f)
    Call LoadJacobianCol2D(rear, gamma, iElementNode, rDeltaGamma, f, F0)
    Call RestoreSurface(rear) 'Restores surface Gamma, Psi, and Recomb
Next iElementNode
Call CalculateB

'Rear Psi
rDeltaPsi = FunctionDeltaPsi
For iElementNode = 1 To nElementNodesX
    Call saveSurface(rear)    'Saves surface Gamma,Psi, and Recomb
    Call IncrementVariable2D(rear, psi, iElementNode, rDeltaPsi) 'Perturbs Psi only
    Call CalculateB
    Call CalculateSurfaceGradGammaPsi2D
    Call ElementFluxErrors2D(f)
    Call LoadJacobianCol2D(rear, psi, iElementNode, rDeltaPsi, f, F0)
    Call RestoreSurface(rear) 'Restores surface Gamma, Psi, and Recomb
Next iElementNode
Call CalculateB

negativeDeltas = Application.MMult(Application.MInverse(eJacobian), F0)
Call UnbundleDeltas2D(negativeDeltas, eDelta)

'Interpolate this update across the surface nodes within each element on both surfaces, for both carriers
For iSurface = front To rear: For iVariable = gamma To psi
    For iElement = 1 To nElements
        Call InterpolateDeltas2D(iSurface, iVariable, iElement, eDelta, True)  'True means damping is applied
    Next iElement
Next iVariable: Next iSurface

bGamma = False: bPsi = False: bRecombination = False: bGradGammaPsi = False

End Sub

Sub IncrementVariable2D(ByVal nSurface As Integer, ByVal nVariable As Integer, _
                        ByVal xElementNode As Integer, ByVal deltaValue As Double)
'Utilizes previous assignment of nodes to elements that was performed in MapSurfaceParameters
'Interpolate this increment across all nodes in the elements touching the incremented element node
'Update Recomb at affected nodes only, and only if nVariable = gamma (psi does not affect surface recombination)
Dim eDelta As Variant
ReDim eDelta(front To rear, gamma To psi, 1 To nElementNodes) As Double   'All entries are zero,
eDelta(nSurface, nVariable, xElementNode) = deltaValue                    'except this one
If xElementNode > 1 Then 'Left Element
    Call InterpolateDeltas2D(nSurface, nVariable, xElementNode - 1, eDelta, False)  'False means no damping
    If nVariable = gamma Then Call UpdateElementRecombination2D(nSurface, xElementNode - 1)
End If
If xElementNode < nElementNodes Then  'Right Element
    Call InterpolateDeltas2D(nSurface, nVariable, xElementNode, eDelta, False)  'False means no damping
    If nVariable = gamma Then Call UpdateElementRecombination2D(nSurface, xElementNode)
End If
End Sub

Sub InterpolateDeltas2D(ByVal nSurface As Integer, ByVal nVariable As Integer, _
                        ByVal elementX As Integer, ByRef eDelta As Variant, ByVal bDamping As Boolean)
'Interpolates the delta for the designated variable across all nodes within the specified element, across full length
'Updates to Psi are dampened to not exceed rVt, to increase convergence robustness.
'Updated Gammas are prevented from going negative
Dim i As Integer, j As Integer
Dim eXleft As Integer, eXright As Integer
Dim deltaX As Double, xOrigin As Double
Dim c0 As Double, c1 As Double
Dim dx As Double, localDelta As Double, newGamma As Double
eXleft = elementX: eXright = elementX + 1
If elementX = 1 Then
    xOrigin = elementBoundariesX(nSurface, elementX) / 2
    deltaX = elementBoundariesX(nSurface, elementX)
Else
    xOrigin = (elementBoundariesX(nSurface, elementX - 1) + elementBoundariesX(nSurface, elementX)) / 2
    deltaX = (elementBoundariesX(nSurface, elementX) - elementBoundariesX(nSurface, elementX - 1))
End If
c0 = (eDelta(nSurface, nVariable, eXright) + eDelta(nSurface, nVariable, eXleft)) / 2
c1 = (eDelta(nSurface, nVariable, eXright) - eDelta(nSurface, nVariable, eXleft)) / deltaX
For i = eElementLowerX(nSurface, elementX) To eElementUpperX(nSurface, elementX)
    For j = 1 To nNodesY
        dx = (i - 1) * 10 * deviceWidth / (nNodesX - 1) - xOrigin  'This routine uses mm
        localDelta = c0 + c1 * dx
        If nSurface = front Then
            If nVariable = gamma Then
                newGamma = sFrontGamma(i, j) + localDelta
                If newGamma <= 0 Then sFrontGamma(i, j) = sFrontGamma(i, j) / 2 Else sFrontGamma(i, j) = newGamma
            Else
                If bDamping Then localDelta = localDelta * rVt / (rVt + Abs(localDelta))
                sFrontPsi(i, j) = sFrontPsi(i, j) + localDelta
            End If
        Else
            If nVariable = gamma Then
                newGamma = sRearGamma(i, j) + localDelta
                If newGamma <= 0 Then sRearGamma(i, j) = sRearGamma(i, j) / 2 Else sRearGamma(i, j) = newGamma
            Else
                If bDamping Then localDelta = localDelta * rVt / (rVt + Abs(localDelta))
                sRearPsi(i, j) = sRearPsi(i, j) + localDelta
            End If
        End If
        sPhi(front, holes, i, j) = PhiP(sFrontGamma(i, j), sFrontPsi(i, j))
        sPhi(front, electrons, i, j) = PhiN(sFrontGamma(i, j), sFrontPsi(i, j))
        sPhi(rear, holes, i, j) = PhiP(sRearGamma(i, j), sRearPsi(i, j))
        sPhi(rear, electrons, i, j) = PhiN(sRearGamma(i, j), sRearPsi(i, j))
    Next j
Next i
End Sub

Sub ElementFluxErrors2D(ByRef eErrorF As Variant)
'Returns flux imbalance for the region surrounding each element node (times q to give A/cm2)
'as a vector ordered front to rear, holes to electrons, xElementNode
'The imbalance is the element-region average of nodal FLUX error
Const nElementNodesX = nElementNodes
Dim i As Integer, iElementNode As Integer, nPointer As Integer
Dim sFluxImbalance As Variant
ReDim sFluxImbalance(front To rear, holes To electrons, 1 To nNodesX, 1 To 1) As Double

Call CalculateNodalFluxErrors(nNodesX, 1, sFluxImbalance)

nPointer = 0
For iElementNode = 1 To nElementNodesX
    eErrorF(nPointer + iElementNode, 1) = _
        RegionAverage2D(front, holes, iElementNode, sFluxImbalance)
Next iElementNode
nPointer = nPointer + nElementNodesX
For iElementNode = 1 To nElementNodesX
    eErrorF(nPointer + iElementNode, 1) = _
        RegionAverage2D(front, electrons, iElementNode, sFluxImbalance)
Next iElementNode
nPointer = nPointer + nElementNodesX
For iElementNode = 1 To nElementNodesX
    eErrorF(nPointer + iElementNode, 1) = _
        RegionAverage2D(rear, holes, iElementNode, sFluxImbalance)
Next iElementNode
nPointer = nPointer + nElementNodesX
For iElementNode = 1 To nElementNodesX
    eErrorF(nPointer + iElementNode, 1) = _
        RegionAverage2D(rear, electrons, iElementNode, sFluxImbalance)
Next iElementNode

End Sub

Sub LoadJacobianCol2D(ByVal nSurface As Integer, ByVal nVariable As Integer, _
                      ByVal xElementNode As Integer, _
                      ByVal deltaValue As Double, ByRef f As Variant, ByRef F0 As Variant)
'This routine loads one column of the Jacobian matrix designated by surface and variable
Const nElementNodesX = nElementNodes
Dim row As Integer, col As Integer, i As Integer
Dim iSurface As Integer, iCarrier As Integer
col = (2 * (nSurface - 1) + (nVariable - 1)) * nElementNodesX + xElementNode
row = 0
For iSurface = front To rear: For iCarrier = holes To electrons
    For i = 1 To nElementNodesX
        row = row + 1
        eJacobian(row, col) = (f(row, 1) - F0(row, 1)) / deltaValue
    Next i
Next iCarrier: Next iSurface
End Sub

Sub UnbundleDeltas2D(ByRef negativeDeltas As Variant, ByRef eDelta As Variant)
'Parses the vector negativeDeltas to extract values for the two variables at the element nodes on each surface
Const nElementNodesX = nElementNodes
Dim i As Integer, nPointer As Integer
Dim iSurface As Integer, iVariable As Integer
nPointer = 0
For i = 1 To nElementNodesX
    eDelta(front, gamma, i) = -negativeDeltas(nPointer + i, 1)
Next i
nPointer = nPointer + nElementNodesX
For i = 1 To nElementNodesX
    eDelta(front, psi, i) = -negativeDeltas(nPointer + i, 1)
Next i
nPointer = nPointer + nElementNodesX
For i = 1 To nElementNodesX
    eDelta(rear, gamma, i) = -negativeDeltas(nPointer + i, 1)
Next i
nPointer = nPointer + nElementNodesX
For i = 1 To nElementNodesX
    eDelta(rear, psi, i) = -negativeDeltas(nPointer + i, 1)
Next i
End Sub

Sub UpdateElementRecombination2D(ByVal nSurface As Integer, ByVal xElement As Integer)
'Updates the surface recombination only at nodes lying within the designated element, and only at j=1 since 2D.
Dim i As Integer
For i = eElementLowerX(nSurface, xElement) To eElementUpperX(nSurface, xElement)
    If nSurface = front Then
        sFrontRecomb(i, 1) = SurfaceRecombinationJo1(front, i, 1, sFrontGamma(i, 1)) _
        + SurfaceRecombinationJo2(front, i, 1, sFrontGamma(i, 1))   'Includes Gsh
    Else
        sRearRecomb(i, 1) = SurfaceRecombinationJo1(rear, i, 1, sRearGamma(i, 1)) _
        + SurfaceRecombinationJo2(rear, i, 1, sRearGamma(i, 1))     'Includes Gsh
    End If
Next i
End Sub

Function RegionAverage2D(ByVal nSurface As Integer, ByVal nCarrier As Integer, ByVal xRegion As Integer, _
                         ByRef sArray As Variant) As Double
'Calculates the average value of sArray on the designated surface for nodes surrounding xElementNode, at j=1.
'The region is defined by previously set Node Allocation matrices
'The sArray is ordered front to rear, holes to electrons, 1 to nNodesX, 1 to 1
Dim i As Integer
Dim regionTotal As Double, areaTotal As Double, localWeight As Double
regionTotal = 0: areaTotal = 0
For i = eRegionLowerX(nSurface, xRegion) To eRegionUpperX(nSurface, xRegion)
        localWeight = eNodeAllocationX(nSurface, i, xRegion)
        areaTotal = areaTotal + localWeight
        regionTotal = regionTotal + localWeight * sArray(nSurface, nCarrier, i, 1)
Next i
If areaTotal > 0 Then
    RegionAverage2D = regionTotal / areaTotal
    Else: RegionAverage2D = 0
End If
End Function

