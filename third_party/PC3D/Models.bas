Attribute VB_Name = "Models"
Option Explicit
'When a model modifies a parameter, include code in Batch to restore the modified values.

Sub TemperatureModel()
Dim oldT As Double, newT As Double, unRounded As Double
Dim deltaE As Double, oldLambda As Double, newLambda As Double
Dim a As Double, b As Double, c As Double
Dim x1 As Double, x2 As Double, x3 As Double
Dim y1 As Double, y2 As Double, y3 As Double
Dim i As Integer, j As Integer
oldT = rTemperature
newT = Range(temperatureCell) + rToffset
Call MapSurfaceParameters   'Necessary in case edits have been made but not yet mapped. Caution: changes rTemperature.
'Obtain intrinsic concentration from parabolic fit of ln(ni) vs 1/T
x1 = 1 / Range(modelTempNiCells).Cells(1, 1)
x2 = 1 / Range(modelTempNiCells).Cells(2, 1)
x3 = 1 / Range(modelTempNiCells).Cells(3, 1)
y1 = Log(Range(modelTempNiCells).Cells(1, 2))
y2 = Log(Range(modelTempNiCells).Cells(2, 2))
y3 = Log(Range(modelTempNiCells).Cells(3, 2))
a = ((y3 - y2) * (x2 - x1) - (y2 - y1) * (x3 - x2)) / ((x3 ^ 2 - x2 ^ 2) * (x2 - x1) - (x2 ^ 2 - x1 ^ 2) * (x3 - x2))
b = ((y2 - y1) - a * (x2 ^ 2 - x1 ^ 2)) / (x2 - x1)
c = y1 - a * x1 ^ 2 - b * x1
Range(intrinsicCell).Value = Exp(a * (1 / newT) ^ 2 + b * (1 / newT) + c)
'Shift absorption coefficients according to change in bandgap
deltaE = -0.001 * Range(modelTempOpticsCells).Cells(1, 1) _
    * (newT ^ 2 / (newT + Range(modelTempOpticsCells).Cells(2, 1)) - oldT ^ 2 / (oldT + Range(modelTempOpticsCells).Cells(2, 1)))
For i = 1 To UBound(rSpectrumData)
    oldLambda = rSpectrumData(i, 1)
    If Not (oldLambda > 0) Then Exit For
    newLambda = oldLambda / (1 - deltaE * oldLambda / hc)
    Range(spectrumDataCells).Cells(i, 3) = InterpolateColumns(newLambda, 1, rSpectrumData, 3, rSpectrumData, True, False)
Next i
'Adjust bulk recombination lifetimes
If Range(tauNUnitsCell) = "ms" Then
    Range(tauNCell).Value = Round(1000# * rTauN * (newT / oldT) ^ Range(modelTempExponentCell), 2)
Else
    Range(tauNCell).Value = Round(1000000# * rTauN * (newT / oldT) ^ Range(modelTempExponentCell), 2)
End If
If Range(tauPUnitsCell) = "ms" Then
    Range(tauPCell).Value = Round(1000# * rTauP * (newT / oldT) ^ Range(modelTempExponentCell), 2)
Else
    Range(tauPCell).Value = Round(1000000# * rTauP * (newT / oldT) ^ Range(modelTempExponentCell), 2)
End If
'Adjust surface recombination values
For i = 1 To nRows: For j = 1 To nColumns
    unRounded = rFrontJo1(i, j) * (Range(intrinsicCell) / rNi) ^ Range(modelTempSurfaceCells).Cells(1, 1)
      If unRounded > 0 Then Range(frontJo1Cells).Cells(i, j) = Round(unRounded, WorksheetFunction.max(0, 4 - WorksheetFunction.Ceiling(Log(unRounded) / 2.3, 1)))
    unRounded = rRearJo1(i, j) * (Range(intrinsicCell) / rNi) ^ Range(modelTempSurfaceCells).Cells(1, 1)
      If unRounded > 0 Then Range(rearJo1Cells).Cells(i, j) = Round(unRounded, WorksheetFunction.max(0, 4 - WorksheetFunction.Ceiling(Log(unRounded) / 2.3, 1)))
    unRounded = rFrontJo2(i, j) * (Range(intrinsicCell) / rNi) ^ Range(modelTempSurfaceCells).Cells(2, 1)
      If unRounded > 0 Then Range(frontJo2Cells).Cells(i, j) = Round(unRounded, WorksheetFunction.max(0, 4 - WorksheetFunction.Ceiling(Log(unRounded) / 2.3, 1)))
    unRounded = rRearJo2(i, j) * (Range(intrinsicCell) / rNi) ^ Range(modelTempSurfaceCells).Cells(2, 1)
      If unRounded > 0 Then Range(rearJo2Cells).Cells(i, j) = Round(unRounded, WorksheetFunction.max(0, 4 - WorksheetFunction.Ceiling(Log(unRounded) / 2.3, 1)))
    unRounded = rFrontGsh(i, j) * (Range(intrinsicCell) / rNi) ^ Range(modelTempSurfaceCells).Cells(3, 1)
      If unRounded > 0 Then Range(frontGshCells).Cells(i, j) = Round(unRounded, WorksheetFunction.max(0, 4 - WorksheetFunction.Ceiling(Log(unRounded) / 2.3, 1)))
    unRounded = rRearGsh(i, j) * (Range(intrinsicCell) / rNi) ^ Range(modelTempSurfaceCells).Cells(3, 1)
      If unRounded > 0 Then Range(rearGshCells).Cells(i, j) = Round(unRounded, WorksheetFunction.max(0, 4 - WorksheetFunction.Ceiling(Log(unRounded) / 2.3, 1)))
Next j: Next i
End Sub

Sub DopingModel()
Dim onset As Double, exponent As Double, rNref As Double, alpha As Double
Dim oldDoping As Double, newDoping As Double, unRounded As Double
Dim adjOldDoping As Double, adjNewDoping As Double
Dim i As Integer, j As Integer
oldDoping = Abs(rDoping)
newDoping = Abs(Range(deviceDopingCell))
Call MapSurfaceParameters   'Necessary in case edits have been made but not yet mapped. Caution: changes rDoping.
'Adjust bulk recombination lifetimes
onset = Range(modelDopingBulkCells).Cells(1, 1)
exponent = Range(modelDopingBulkCells).Cells(2, 1)
If oldDoping > onset Then
    rTauN = rTauN * (oldDoping / onset) ^ -exponent
    rTauP = rTauP * (oldDoping / onset) ^ -exponent
End If
If newDoping < onset Then adjNewDoping = onset Else adjNewDoping = newDoping
If Range(tauNUnitsCell) = "ms" Then
    Range(tauNCell).Value = Round(1000# * rTauN * (adjNewDoping / onset) ^ exponent, 2)
Else
    Range(tauNCell).Value = Round(1000000# * rTauN * (adjNewDoping / onset) ^ exponent, 2)
End If
If Range(tauPUnitsCell) = "ms" Then
    Range(tauPCell).Value = Round(1000# * rTauP * (adjNewDoping / onset) ^ exponent, 2)
Else
    Range(tauPCell).Value = Round(1000000# * rTauP * (adjNewDoping / onset) ^ exponent, 2)
End If
'Adjust Jo1 at undoped surfaces
rNref = Range(modelDopingSurfaceCells).Cells(1, 1)
alpha = Range(modelDopingSurfaceCells).Cells(2, 1)
For i = 1 To nRows: For j = 1 To nColumns
    If rFrontSheetRho(i, j) = 0 And rFrontJo1(i, j) > 0 Then
        unRounded = rFrontJo1(i, j) * (1 + (oldDoping / rNref) ^ alpha) / (1 + (newDoping / rNref) ^ alpha)
        Range(frontJo1Cells).Cells(i, j) = Round(unRounded, WorksheetFunction.max(0, 4 - WorksheetFunction.Ceiling(Log(unRounded) / 2.3, 1)))
    End If
    If rRearSheetRho(i, j) = 0 And rRearJo1(i, j) > 0 Then
        unRounded = rRearJo1(i, j) * (1 + (oldDoping / rNref) ^ alpha) / (1 + (newDoping / rNref) ^ alpha)
        Range(rearJo1Cells).Cells(i, j) = Round(unRounded, WorksheetFunction.max(0, 4 - WorksheetFunction.Ceiling(Log(unRounded) / 2.3, 1)))
    End If
Next j: Next i
End Sub
