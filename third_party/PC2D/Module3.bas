Attribute VB_Name = "Module3"
Sub SolveIV()
' This macro solves an IV curve.  The user inputs a starting and ending voltage, as well as the desired increment.
' Below 80% of V1, the odd increments are skipped
    
    Dim IVStart, IVFinal, IVIncrement, Vcont, V1 As Double
    Dim i, j, n As Integer
    Dim HavePmax, HaveVoc, HaveIsc, Voice, Ramp As Boolean
    
    Range("A1").Select
    Voice = Range("Excitation!T16").Value = "Yes"
    Ramp = Range("Excitation!T11").Value = "Yes"

' Clear previous results
    Range("Excitation!B22:B23").ClearContents
    Range("Excitation!D22:D23").ClearContents
    Range("Current!L25:O526").ClearContents
    
    IVStart = Range("Excitation!B18").Value
    IVFinal = Range("Excitation!B19").Value
    IVIncrement = Range("Excitation!B20").Value
    V1 = Range("Excitation!G5").Value
    SaveVoltage = Range("Excitation!G2").Value

' Error messages for the user.
    If IVIncrement = 0 Then
        MsgBox "You need to specify a non-zero voltage increment.", vbOKOnly, "PC2D Alert"
        Exit Sub
    End If
    
    If (IVFinal - IVStart) / IVIncrement > 500 Then
        MsgBox "You are trying to solve for too many data points.  Please specify a larger voltage increment.", vbOKOnly, "PC2D Alert"
        Exit Sub
    End If

    If Not (Range("Excitation!C11").Value = 0.1) Then
      If MsgBox("You are not solving for one sun intensity.  Would you like to proceed?", vbYesNo, "PC2D Alert") = vbNo Then
        Exit Sub
      End If
    End If
        
    If Not (Range("Excitation!F11").Value = "AM1.5G") Then
       If MsgBox("You are not solving for the AM 1.5G spectrum.  Would you like to proceed?", vbYesNo, "PC2D Alert") = vbNo Then
          Exit Sub
       End If
    End If

' The macro will only scan from low to high.  This will reverse the starting and ending voltages if the starting voltage is greater
' than the ending voltage.
    If IVStart > IVFinal Then
        IVStart = Range("Excitation!B19").Value
        IVFinal = Range("Excitation!B18").Value
        Range("Excitation!B18").Value = IVStart
        Range("Excitation!B19").Value = IVFinal
    End If

    If IVIncrement < 0 Then
        IVIncrement = Abs(IVIncrement)
        Range("Excitation!B20").Value = IVIncrement
    End If

    n = (IVFinal - IVStart) / IVIncrement
        
    Range("Excitation!G2").Value = IVStart
 
 IVout = Range("Current!L25:O526")
 j = 0
 Call ResetElectric
 Call UpdateScreen
   
 For i = 0 To n
      Vcont = IVStart + IVIncrement * i
      If (Vcont > IVFinal - 0.2) Or (i Mod 2 = 0) Then
        j = j + 1
        Range("Excitation!G2").Value = Vcont
        If (i = 0) Then
            If Ramp Then
                Range("Excitation!c11").Value = Range("Excitation!c11").Value / 100
                Call SolveGeneration
                Call SolveElectric
                Range("Excitation!c11").Value = Range("Excitation!c11").Value * 10
                Call SolveGeneration
                Call SolveElectric
                Range("Excitation!c11").Value = Range("Excitation!c11").Value * 10
            End If
            Call SolveGeneration
        End If
        Call SolveElectric
        If Not (Range("Excitation!B8").Value = "Done") Then
          MsgBox "This voltage setting failed to converge.", vbOKOnly, "PC2D Alert"
          Exit For
        End If
        IVout(j + 1, 1) = Vcont
        IVout(j + 1, 2) = Range("Excitation!Q2").Value
        IVout(j + 1, 3) = Range("Excitation!Q3").Value
        IVout(j + 1, 4) = Range("Excitation!Q4").Value
        Range("Current!L25:O526") = IVout
        Call UpdateScreen
       End If
      Next i
       
    HavePmax = SolveMaxP(IVout)
    HaveVoc = SolveVoc(IVout)
    HaveIsc = SolveIsc(IVout)
    If HavePmax And HaveVoc And HaveIsc Then    'Calculate and display Fill Factor
      Range("Excitation!D22").Value = Range("Excitation!D23").Value / (Range("Excitation!B22").Value * Range("Excitation!B23").Value)
    End If
    
' The speech application does not work for Excel for Mac.  This allows the same code to be used for both Excel versions.
    On Error Resume Next
   If Voice Then Application.Speech.Speak "I-V Curve Complete"
       
 If Range("Excitation!B21").Value = "Restore" Then
    If Voice Then Application.Speech.Speak "Restoring previous solution"
    Range("Excitation!G2").Value = SaveVoltage
    Call SolveElectric
    If Not (Range("Excitation!B8").Value = "Done") Then MsgBox "Previous solution failed to converge.", vbOKOnly, "PC2D Alert"
 End If
 
 If Range("Excitation!B21").Value = "Voc" Then
    If SolveVoc(IVout) Then
      Range("Excitation!G2").Value = Range("Excitation!B23").Value
      If Voice Then Application.Speech.Speak "Open circuit"
      Call SolveElectric
      If Not (Range("Excitation!B8").Value = "Done") Then MsgBox "Open circuit failed to converge.", vbOKOnly, "PC2D Alert"
      Range("Current!L25:O526") = IVout
    Else: Application.Speech.Speak "V-o-c not found"
    End If
 End If
    
 If Range("Excitation!B21").Value = "MaxPower" Then
    If SolveMaxP(IVout) Then
      If Voice Then Application.Speech.Speak "Max power"
      Call SolveElectric
      If Not (Range("Excitation!B8").Value = "Done") Then MsgBox "Max Power failed to converge.", vbOKOnly, "PC2D Alert"
      IVout(1, 1) = Range("Excitation!G2").Value
      IVout(1, 2) = Range("Excitation!Q2").Value
      IVout(1, 3) = Range("Excitation!Q3").Value
      IVout(1, 4) = Range("Excitation!Q4").Value
      Range("Current!L25:O526") = IVout
    Else
      If Voice Then Application.Speech.Speak "Max power not found"
    End If
 End If
Range("Current!A1").Select
End Sub

Function SolveMaxP(IVout)
    Dim Row, MaxRow As Integer
    Dim Pmax, Pzero, Pplus, Pminus As Double
    Dim Vmax, Vzero, Vplus, minus As Double
    
    MaxRow = 1
    Pmax = 0
    Vmax = 0
 
    For Row = 2 To 502
        If IVout(Row, 4) > Pmax Then
          MaxRow = Row
          Pmax = IVout(Row, 4)
          Vmax = IVout(Row, 1)
        End If
        Next Row
    
    SolveMaxP = False
    If MaxRow > 2 Then If IVout(MaxRow - 1, 4) <> 0 Then If MaxRow < 502 Then If IVout(MaxRow + 1, 4) <> 0 Then SolveMaxP = True
    If Not SolveMaxP Then Exit Function
    
    Pzero = IVout(MaxRow, 4)
    Pplus = IVout(MaxRow + 1, 4)
    Pminus = IVout(MaxRow - 1, 4)
    
    Vzero = IVout(MaxRow, 1)
    Vplus = IVout(MaxRow + 1, 1)
    Vminus = IVout(MaxRow - 1, 1)
    
    If 2 * Pzero - Pplus - Pminus > 0 Then
      Pmax = Pzero + (((Pplus - Pminus) ^ 2) / 8) / (2 * Pzero - Pplus - Pminus)
      Range("Excitation!D23").Value = Pmax
      Vmax = Vzero + (Vplus - Vminus) * ((Pplus - Pminus) / 4) / (2 * Pzero - Pplus - Pminus)
      Range("Excitation!G2").Value = Vmax
    End If
      
End Function

Sub SolveSpectral()
    Dim EQEStart As Integer
    Dim EQEFinal As Integer
    Dim EQEIncrement As Integer
    Dim EQECount As Integer
    Dim EQEn As Integer
    Dim n As Integer
    Dim Voice As Boolean

    Range("A1").Select
    Voice = Range("Excitation!T16").Value = "Yes"
    
    Range("Current!Q25:S525").ClearContents
    SpectralOut = Range("Current!Q25:S525")
    
    SaveLambda = Range("Excitation!I11").Value
    SaveSpectrum = Range("Excitation!F11").Value
    SaveVoltage = Range("Excitation!G2").Value
    EQEStart = Range("Excitation!I18").Value
    EQEFinal = Range("Excitation!I19").Value
    EQEIncrement = Range("Excitation!I20").Value

    If EQEIncrement = 0 Then
        MsgBox "You need to specify a non-zero wavelength increment.", vbOKOnly, "PC2D Alert"
        Exit Sub
    End If
    
    If (EQEFinal - EQEStart) / EQEIncrement > 500 Then
        MsgBox "You are trying to solve for too many data points.  Please select a larger wavelength increment.", vbOKOnly, "PC2D Alert"
        Exit Sub
    End If
    
    If Range("Excitation!C11").Value = 0 Then
        MsgBox "You have chosen zero intensity.  Please select desired intensity to proceed.", vbOKOnly, "PC2D Alert"
        Exit Sub
    End If

    Range("Excitation!G2").Value = 0
    Range("Excitation!F11").Value = "Mono"
    
    If EQEStart > EQEFinal Then
        EQEStart = Range("Excitation!I19").Value
        EQEFinal = Range("Excitation!I18").Value
        Range("Excitation!I18").Value = EQEStart
        Range("Excitation!I19").Value = EQEFinal
    End If

    If EQEIncrement < 0 Then
        EQEIncrement = Abs(EQEIncrement)
        Range("Excitation!I20").Value = EQEIncrement
    End If

    EQEn = (EQEFinal - EQEStart) / EQEIncrement

 Call ResetElectric
 Call UpdateScreen
 
 For n = 0 To EQEn
     EQECount = EQEStart + EQEIncrement * n
     Range("Excitation!I11").Value = EQECount
     Call SolveGeneration
     Call SolveElectric
     SpectralOut(n + 1, 1) = EQECount
     SpectralOut(n + 1, 2) = Range("Excitation!O11")
     SpectralOut(n + 1, 3) = Range("Excitation!Q11")
     Range("Current!q25:s525") = SpectralOut
     Call UpdateScreen
 Next n
      
 Range("Excitation!I11").Value = SaveLambda
 Range("Excitation!F11").Value = SaveSpectrum
 Range("Excitation!G2").Value = SaveVoltage
 
 On Error Resume Next
 If Voice Then Application.Speech.Speak "Spectral response complete"
        
 If Voice Then Application.Speech.Speak "Restoring previous solution"
 Call SolveGeneration
 Call ResetElectric
 Call SolveElectric

Range("Current!A1").Select
End Sub

Sub SolveSpatial()
    
  Dim n As Integer
  Dim Voice As Boolean
  Dim SaveVoltage As Double

  Range("A1").Select
  Voice = Range("Excitation!T16").Value = "Yes"
    
  Range("Current!U25:U44").ClearContents
  SpatialOut = Range("Current!U25:U44")
    
  SaveSpatial = Range("Excitation!b14:u14")
  SaveVoltage = Range("Excitation!G2").Value
    
  If Range("Excitation!C11").Value = 0 Then
      MsgBox "You have chosen zero illumination intensity.  Please select desired intensity to proceed.", vbOKOnly, "PC2D Alert"
      Exit Sub
  End If

  If Range("Excitation!F11").Value = "None" Then
      MsgBox "You have not selected an illumination spectrum.  Please select the desired spectrum.", vbOKOnly, "PC2D Alert"
      Exit Sub
  End If
    
  If Range("Excitation!F11").Value = "Mono" Then
      MsgBox "You are running the spatial response scan with monochromatic light.", vbOKOnly, "PC2D Alert"
  End If
    
  Range("Excitation!G2").Value = 0
  Range("Excitation!b14:u14").ClearContents
  SpatialMultiplier = Range("Excitation!b14:u14")
    
  Call ResetElectric
  Call UpdateScreen
 
  For n = 1 To Num
    SpatialMultiplier(1, n) = 1
    Range("Excitation!b14:u14") = SpatialMultiplier
    Call ResetElectric
    Call SolveGeneration
    Call SolveElectric
    SpatialOut(n, 1) = Range("Excitation!Q3")
    Range("Current!U25:U44") = SpatialOut
    Range("Excitation!b14:u14").ClearContents
    SpatialMultiplier(1, n) = 0
    Call UpdateScreen
  Next n
      
  On Error Resume Next
  If Voice Then Application.Speech.Speak "Spatial response complete"
        
  Range("Excitation!G2").Value = SaveVoltage
  Range("Excitation!b14:u14") = SaveSpatial
 
  If Voice Then Application.Speech.Speak "Restoring previous solution"
  Call SolveGeneration
  Call ResetElectric
  Call SolveElectric

Range("Current!A1").Select
End Sub

Sub ClearGraphs()
    
    Range("Current!L25:U526").ClearContents
    Range("Excitation!B22:B23").ClearContents
    Range("Excitation!D22:D23").ClearContents

End Sub

Function SolveVoc(IVout) As Boolean
    Dim Vminus, Iminus, Vplus, Iplus, V1, V2, V3, I1, I2, I3, Demoninator, A, B, C, Voc As Double
    Dim Row, Rminus, Rplus As Integer
        
    Rminus = 502
    Rplus = 2
    Voc = 0
    
    For Row = 2 To 502
        If (IVout(Row, 3) > 0) And (IVout(Row, 2) <> 0) Then
            Rminus = Row
            End If
        Next Row
    
    For Row = 502 To 2 Step -1
        If (IVout(Row, 3) <= 0) And (IVout(Row, 2) <> 0) Then
            Rplus = Row
            End If
        Next Row
    
    SolveVoc = False
    If Not (Rplus > Rminus) Then Exit Function
   
    If Rminus = 2 Then  'Only one point left of Voc, so use linear interpolation
      Vminus = IVout(Rminus, 2)
      Vplus = IVout(Rplus, 2)
      Iminus = IVout(Rminus, 3)
      Iplus = IVout(Rplus, 3)
      If (Iplus - Iminus) < 0 Then Voc = (Vminus * Iplus - Vplus * Iminus) / (Iplus - Iminus) Else Exit Function
    Else                'Use parabolic interpolation
      V1 = IVout(Rminus - 1, 2)
      V2 = IVout(Rminus, 2)
      V3 = IVout(Rplus, 2)
      I1 = IVout(Rminus - 1, 3)
      I2 = IVout(Rminus, 3)
      I3 = IVout(Rplus, 3)
      Denominator = (V1 ^ 2 - V2 ^ 2) * (V2 - V3) - (V2 ^ 2 - V3 ^ 2) * (V1 - V2)
      If Denominator <> 0 Then A = ((V2 - V3) * (I1 - I2) - (V1 - V2) * (I2 - I3)) / Denominator Else Exit Function
      If V1 <> V2 Then B = ((I1 - I2) - A * (V1 ^ 2 - V2 ^ 2)) / (V1 - V2) Else Exit Function
      C = I1 - B * V1 - A * V1 ^ 2
      If A <> 0 Then Voc = (-B - Sqr(B ^ 2 - 4 * A * C)) / (2 * A) Else Exit Function
    End If
    
    SolveVoc = True
    Range("Excitation!B23").Value = Voc
        
End Function

Function SolveIsc(IVout) As Boolean
    Dim Vminus, Iminus, Vplus, Iplus, Isc As Double
    Dim Row, Rminus, Rplus As Integer
        
    Rminus = 502
    Rplus = 2
    Isc = 0
    
    For Row = 2 To 502
        If (IVout(Row, 2) <= 0) And (IVout(Row, 3) <> 0) Then
            Rminus = Row
            End If
        Next Row
    
    For Row = 502 To 2 Step -1
        If (IVout(Row, 2) > 0) And (IVout(Row, 3) <> 0) Then
            Rplus = Row
            End If
        Next Row
    
    SolveIsc = False
    If Not (Rplus > Rminus) Then Exit Function
   
    Vminus = IVout(Rminus, 2)
    Vplus = IVout(Rplus, 2)
    Iminus = IVout(Rminus, 3)
    Iplus = IVout(Rplus, 3)
    If (Vplus - Vminus) > 0 Then Isc = (Vplus * Iminus - Vminus * Iplus) / (Vplus - Vminus) Else Exit Function
    
    SolveIsc = True
    Range("Excitation!B22").Value = Isc
        
End Function

Sub UpdateScreen()  'This is an absurd kludge necessitated by some oddity introduced in Excel 2016

   DoEvents
   DoEvents
   DoEvents
   
End Sub
