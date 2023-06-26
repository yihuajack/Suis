Attribute VB_Name = "Adjust3D"
Option Explicit

Sub AdjustSurfaces3D()
'This routine evaluates the effect of perturbing each of these element node's variables on the flux balance
'for holes and electrons within each region. adjusts the values of gamma and psi at the surfaces in an
'attempt to eliminate flux imbalance at the surface nodes.
Const nElementNodesX = nElementNodes, nElementNodesY = nElementNodes
Dim iElementNode As Integer, jElementNode As Integer
Dim iSurface As Integer, iVariable As Integer, iElement As Integer, jElement As Integer
Dim rDeltaGamma As Double, rDeltaPsi As Double
Dim F0 As Variant, f As Variant
Dim eDelta As Variant
Dim negativeDeltas As Variant
ReDim eJacobian(1 To 4 * nElementNodesX * nElementNodesY, 1 To 4 * nElementNodesX * nElementNodesY) As Double
ReDim F0(1 To 4 * nElementNodesX * nElementNodesY, 1 To 1) As Double  'Errors at element nodes before perturbation
ReDim f(1 To 4 * nElementNodesX * nElementNodesY, 1 To 1) As Double   'Errors at element nodes after perturbation
ReDim negativeDeltas(1 To 4 * nElementNodesX * nElementNodesY, 1 To 1) As Double
ReDim eDelta(front To rear, gamma To psi, 1 To nElementNodesX, 1 To nElementNodesY) As Double

Call CalculateSurfaceGradGammaPsi3D
Call ElementFluxErrors3D(F0)     'Set reference (unperturbed) element flux error matrix

'Front Gamma
rDeltaGamma = FunctionDeltaGamma
For iElementNode = 1 To nElementNodesX: For jElementNode = 1 To nElementNodesY
    Call saveSurface(front)    'Saves surface Gamma, Psi, and Recomb
    Call IncrementVariable3D(front, gamma, iElementNode, jElementNode, rDeltaGamma) 'Perturbs Gamma and Recomb
    Call CalculateA
    Call CalculateSurfaceGradGammaPsi3D
    Call ElementFluxErrors3D(f)
    Call LoadJacobianCol3D(front, gamma, iElementNode, jElementNode, rDeltaGamma, f, F0)
    Call RestoreSurface(front)  'Restores surface Gamma, Psi, and Recomb
Next jElementNode: Next iElementNode
Call CalculateA
If b3D Then DoEvents        'For long 3D calculations, these calls keep Windows from thinking the program is not responding.

'Front Psi
rDeltaPsi = FunctionDeltaPsi
For iElementNode = 1 To nElementNodesX: For jElementNode = 1 To nElementNodesY
    Call saveSurface(front)     'Saves surface Gamma, Psi, and Recomb
    Call IncrementVariable3D(front, psi, iElementNode, jElementNode, rDeltaPsi) 'Perturbs Psi only
    Call CalculateA
    Call CalculateSurfaceGradGammaPsi3D
    Call ElementFluxErrors3D(f)
    Call LoadJacobianCol3D(front, psi, iElementNode, jElementNode, rDeltaPsi, f, F0)
    Call RestoreSurface(front) 'Restores surface Gamma, Psi, and Recomb
Next jElementNode: Next iElementNode
Call CalculateA
If b3D Then DoEvents

'Rear Gamma
rDeltaGamma = FunctionDeltaGamma
For iElementNode = 1 To nElementNodesX: For jElementNode = 1 To nElementNodesY
    Call saveSurface(rear)    'Saves surface Gamma,Psi, and Recomb
    Call IncrementVariable3D(rear, gamma, iElementNode, jElementNode, rDeltaGamma)   'Perturbs Gamma and Recomb
    Call CalculateB
    Call CalculateSurfaceGradGammaPsi3D
    Call ElementFluxErrors3D(f)
    Call LoadJacobianCol3D(rear, gamma, iElementNode, jElementNode, rDeltaGamma, f, F0)
    Call RestoreSurface(rear) 'Restores surface Gamma, Psi, and Recomb
Next jElementNode: Next iElementNode
Call CalculateB
If b3D Then DoEvents

'Rear Psi
rDeltaPsi = FunctionDeltaPsi
For iElementNode = 1 To nElementNodesX: For jElementNode = 1 To nElementNodesY
    Call saveSurface(rear)    'Saves surface Gamma,Psi, and Recomb
    Call IncrementVariable3D(rear, psi, iElementNode, jElementNode, rDeltaPsi) 'Perturbs Psi only
    Call CalculateB
    Call CalculateSurfaceGradGammaPsi3D
    Call ElementFluxErrors3D(f)
    Call LoadJacobianCol3D(rear, psi, iElementNode, jElementNode, rDeltaPsi, f, F0)
    Call RestoreSurface(rear) 'Restores surface Gamma, Psi, and Recomb
Next jElementNode: Next iElementNode
Call CalculateB
If b3D Then DoEvents

negativeDeltas = Application.MMult(Application.MInverse(eJacobian), F0)
Call UnbundleDeltas3D(negativeDeltas, eDelta)

'Interpolate this update across the surface nodes within each element on both surfaces, for both carriers
For iSurface = front To rear: For iVariable = gamma To psi
    For iElement = 1 To nElements: For jElement = 1 To nElements
        Call InterpolateDeltas3D(iSurface, iVariable, iElement, jElement, eDelta, True) 'True means damping is applied
    Next jElement: Next iElement
Next iVariable: Next iSurface

bGamma = False: bPsi = False: bRecombination = False: bGradGammaPsi = False

End Sub

Sub IncrementVariable3D(ByVal nSurface As Integer, ByVal nVariable As Integer, _
                        ByVal xElementNode As Integer, ByVal yElementNode As Integer, ByVal deltaValue As Double)
'Utilizes previous assignment of nodes to elements that was performed in MapSurfaceParameters
'Interpolate this increment across all nodes in the elements touching the incremented element node
'Update Recomb at affected nodes only and only if nVariable = gamma (psi does not affect surface recombination)
Dim eDelta As Variant
ReDim eDelta(front To rear, gamma To psi, 1 To nElementNodes, 1 To nElementNodes) As Double   'All entries are zero,
eDelta(nSurface, nVariable, xElementNode, yElementNode) = deltaValue                          'except this one
If xElementNode > 1 And yElementNode > 1 Then 'Upper Left Element
    Call InterpolateDeltas3D(nSurface, nVariable, xElementNode - 1, yElementNode - 1, eDelta, False)    'No damping
    If nVariable = gamma Then Call UpdateElementRecombination3D(nSurface, xElementNode - 1, yElementNode - 1)
End If
If xElementNode < nElementNodes And yElementNode > 1 Then 'Upper Right Element
    Call InterpolateDeltas3D(nSurface, nVariable, xElementNode, yElementNode - 1, eDelta, False)    'No damping
    If nVariable = gamma Then Call UpdateElementRecombination3D(nSurface, xElementNode, yElementNode - 1)
End If
If xElementNode > 1 And yElementNode < nElementNodes Then 'Lower Left Element
    Call InterpolateDeltas3D(nSurface, nVariable, xElementNode - 1, yElementNode, eDelta, False)    'No damping
    If nVariable = gamma Then Call UpdateElementRecombination3D(nSurface, xElementNode - 1, yElementNode)
End If
If xElementNode < nElementNodes And yElementNode < nElementNodes Then 'Lower Right Element
    Call InterpolateDeltas3D(nSurface, nVariable, xElementNode, yElementNode, eDelta, False)    'No damping
    If nVariable = gamma Then Call UpdateElementRecombination3D(nSurface, xElementNode, yElementNode)
End If
End Sub

Sub InterpolateDeltas3D(ByVal nSurface As Integer, ByVal nVariable As Integer, _
                        ByVal elementX As Integer, ByVal elementY As Integer, _
                        ByRef eDelta As Variant, ByVal bDamping As Boolean)
'Interpolates the delta for the designated variable across all nodes within the specified element
'Updates to Psi are dampened to not exceed rVt, to increase convergence robustness.
'Updated Gammas are prevented from going negative.
Dim i As Integer, j As Integer
Dim eXleft As Integer, eXright As Integer, eYtop As Integer, eYbot As Integer   'Element nodes
Dim deltaX As Double, deltaY As Double, xOrigin As Double, yOrigin As Double
Dim c0 As Double, c1 As Double, c2 As Double, c3 As Double
Dim dx As Double, dy As Double, localDelta As Double, newGamma As Double
eXleft = elementX: eXright = elementX + 1
If elementX = 1 Then
    xOrigin = elementBoundariesX(nSurface, elementX) / 2
    deltaX = elementBoundariesX(nSurface, elementX)
Else
    xOrigin = (elementBoundariesX(nSurface, elementX - 1) + elementBoundariesX(nSurface, elementX)) / 2
    deltaX = (elementBoundariesX(nSurface, elementX) - elementBoundariesX(nSurface, elementX - 1))
End If
eYtop = elementY: eYbot = elementY + 1
If elementY = 1 Then
    yOrigin = elementBoundariesY(nSurface, elementY) / 2
    deltaY = elementBoundariesY(nSurface, elementY)
Else
    yOrigin = (elementBoundariesY(nSurface, elementY - 1) + elementBoundariesY(nSurface, elementY)) / 2
    deltaY = (elementBoundariesY(nSurface, elementY) - elementBoundariesY(nSurface, elementY - 1))
End If
c0 = (eDelta(nSurface, nVariable, eXright, eYbot) + eDelta(nSurface, nVariable, eXleft, eYbot) _
    + eDelta(nSurface, nVariable, eXright, eYtop) + eDelta(nSurface, nVariable, eXleft, eYtop)) / 4
c1 = (eDelta(nSurface, nVariable, eXright, eYbot) - eDelta(nSurface, nVariable, eXleft, eYbot) _
    + eDelta(nSurface, nVariable, eXright, eYtop) - eDelta(nSurface, nVariable, eXleft, eYtop)) / (2 * deltaX)
c2 = (eDelta(nSurface, nVariable, eXright, eYbot) + eDelta(nSurface, nVariable, eXleft, eYbot) _
    - eDelta(nSurface, nVariable, eXright, eYtop) - eDelta(nSurface, nVariable, eXleft, eYtop)) / (2 * deltaY)
c3 = (eDelta(nSurface, nVariable, eXright, eYbot) - eDelta(nSurface, nVariable, eXleft, eYbot) _
    - eDelta(nSurface, nVariable, eXright, eYtop) + eDelta(nSurface, nVariable, eXleft, eYtop)) / (deltaX * deltaY)
For i = eElementLowerX(nSurface, elementX) To eElementUpperX(nSurface, elementX)
    For j = eElementLowerY(nSurface, elementY) To eElementUpperY(nSurface, elementY)
        dx = (i - 1) * 10 * deviceWidth / (nNodesX - 1) - xOrigin  'This routine uses mm
        dy = (j - 1) * 10 * deviceLength / (nNodesY - 1) - yOrigin 'This routine uses mm
        localDelta = c0 + c1 * dx + c2 * dy + c3 * dx * dy
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

Sub ElementFluxErrors3D(ByRef eErrorF As Variant)
'Returns flux imbalance for the region surrounding each element node (times q to give A/cm2)
'as a vector ordered front to rear, holes to electrons, xElementNode, yElementNode
'The imbalance is the element-region average of nodal FLUX error
Const nElementNodesX = nElementNodes, nElementNodesY = nElementNodes
Dim i As Integer, j As Integer, iElementNode As Integer, jElementNode As Integer, nPointer As Integer
Dim sFluxImbalance As Variant
ReDim sFluxImbalance(front To rear, holes To electrons, 1 To nNodesX, 1 To nNodesY) As Double

Call CalculateNodalFluxErrors(nNodesX, nNodesY, sFluxImbalance)

nPointer = 0
For iElementNode = 1 To nElementNodesX: For jElementNode = 1 To nElementNodesY
    eErrorF(nPointer + (iElementNode - 1) * nElementNodesY + jElementNode, 1) = _
        RegionAverage3D(front, holes, iElementNode, jElementNode, sFluxImbalance)
Next jElementNode: Next iElementNode
nPointer = nPointer + nElementNodesX * nElementNodesY
For iElementNode = 1 To nElementNodesX: For jElementNode = 1 To nElementNodesY
    eErrorF(nPointer + (iElementNode - 1) * nElementNodesY + jElementNode, 1) = _
        RegionAverage3D(front, electrons, iElementNode, jElementNode, sFluxImbalance)
Next jElementNode: Next iElementNode
nPointer = nPointer + nElementNodesX * nElementNodesY
For iElementNode = 1 To nElementNodesX: For jElementNode = 1 To nElementNodesY
    eErrorF(nPointer + (iElementNode - 1) * nElementNodesY + jElementNode, 1) = _
        RegionAverage3D(rear, holes, iElementNode, jElementNode, sFluxImbalance)
Next jElementNode: Next iElementNode
nPointer = nPointer + nElementNodesX * nElementNodesY
For iElementNode = 1 To nElementNodesX: For jElementNode = 1 To nElementNodesY
    eErrorF(nPointer + (iElementNode - 1) * nElementNodesY + jElementNode, 1) = _
        RegionAverage3D(rear, electrons, iElementNode, jElementNode, sFluxImbalance)
Next jElementNode: Next iElementNode

End Sub

Sub LoadJacobianCol3D(ByVal nSurface As Integer, ByVal nVariable As Integer, _
                      ByVal xElementNode As Integer, ByVal yElementNode As Integer, _
                      ByVal deltaValue As Double, ByRef f As Variant, ByRef F0 As Variant)
'This routine loads one column of the Jacobian matrix designated by surface and variable
Const nElementNodesX = nElementNodes, nElementNodesY = nElementNodes
Dim row As Integer, col As Integer, i As Integer, j As Integer
Dim iSurface As Integer, iCarrier As Integer
col = (2 * (nSurface - 1) + (nVariable - 1)) * nElementNodesX * nElementNodesY + (xElementNode - 1) * nElementNodesY + yElementNode
row = 0
For iSurface = front To rear: For iCarrier = holes To electrons
    For i = 1 To nElementNodesX: For j = 1 To nElementNodesY
        row = row + 1
        eJacobian(row, col) = (f(row, 1) - F0(row, 1)) / deltaValue
    Next j: Next i
Next iCarrier: Next iSurface
End Sub

Sub UnbundleDeltas3D(ByRef negativeDeltas As Variant, ByRef eDelta As Variant)
'Parses the vector negativeDeltas to extract values for the two variables at the element nodes on each surface
Const nElementNodesX = nElementNodes, nElementNodesY = nElementNodes
Dim i As Integer, j As Integer, nPointer As Integer
Dim iSurface As Integer, iVariable As Integer
nPointer = 0
For i = 1 To nElementNodesX: For j = 1 To nElementNodesY
    eDelta(front, gamma, i, j) = -negativeDeltas(nPointer + (i - 1) * nElementNodesY + j, 1)
Next j: Next i
nPointer = nPointer + nElementNodesX * nElementNodesY
For i = 1 To nElementNodesX: For j = 1 To nElementNodesY
    eDelta(front, psi, i, j) = -negativeDeltas(nPointer + (i - 1) * nElementNodesY + j, 1)
Next j: Next i
nPointer = nPointer + nElementNodesX * nElementNodesY
For i = 1 To nElementNodesX: For j = 1 To nElementNodesY
    eDelta(rear, gamma, i, j) = -negativeDeltas(nPointer + (i - 1) * nElementNodesY + j, 1)
Next j: Next i
nPointer = nPointer + nElementNodesX * nElementNodesY
For i = 1 To nElementNodesX: For j = 1 To nElementNodesY
    eDelta(rear, psi, i, j) = -negativeDeltas(nPointer + (i - 1) * nElementNodesY + j, 1)
Next j: Next i
End Sub

Sub UpdateElementRecombination3D(ByVal nSurface As Integer, ByVal elementX As Integer, ByVal elementY As Integer)
'Updates surface recombination for all nodes that lie within the specified element
Dim i As Integer, j As Integer
For i = eElementLowerX(nSurface, elementX) To eElementUpperX(nSurface, elementX)
    For j = eElementLowerY(nSurface, elementY) To eElementUpperY(nSurface, elementY)
        If nSurface = front Then
            sFrontRecomb(i, j) = SurfaceRecombinationJo1(front, i, j, sFrontGamma(i, j)) _
        + SurfaceRecombinationJo2(front, i, j, sFrontGamma(i, j))       'Includes Gsh
        Else
            sRearRecomb(i, j) = SurfaceRecombinationJo1(rear, i, j, sRearGamma(i, j)) _
        + SurfaceRecombinationJo2(rear, i, j, sRearGamma(i, j))     'Includes Gsh
        End If
    Next j
Next i
End Sub

Function RegionAverage3D(ByVal nSurface As Integer, ByVal nCarrier As Integer, _
                         ByVal xRegion As Integer, ByVal yRegion As Integer, _
                         ByRef sArray As Variant) As Double
'Finds the region (partial-surface) average of the supplied surface array for the given carrier type
'The region is defined by previously set Node Allocation matrices
'The sArray is ordered front to rear, holes to electrons, 1 to nNodesX, 1 to nNodesY
Dim i As Integer, j As Integer
Dim regionTotal As Double, areaTotal As Double, localWeight As Double
regionTotal = 0: areaTotal = 0
For i = eRegionLowerX(nSurface, xRegion) To eRegionUpperX(nSurface, xRegion)
    For j = eRegionLowerY(nSurface, yRegion) To eRegionUpperY(nSurface, yRegion)
        localWeight = eNodeAllocationX(nSurface, i, xRegion) * eNodeAllocationY(nSurface, j, yRegion)
        areaTotal = areaTotal + localWeight
        regionTotal = regionTotal + localWeight * sArray(nSurface, nCarrier, i, j)
    Next j
Next i
If areaTotal > 0 Then
    RegionAverage3D = regionTotal / areaTotal
    Else: RegionAverage3D = 0
End If
End Function

