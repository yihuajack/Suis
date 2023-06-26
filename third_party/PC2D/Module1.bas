Attribute VB_Name = "Module1"
Public Sub ResetPhi_n()
    Dim Vsource, Doping As Double
    Application.Calculation = xlAutomatic 'Restore recalc
    Vsource = Range("Excitation!g2")
    Doping = Range("Device!c6")
    If Doping > 0 Then
      Range("Phi_n!b3:v3").Value = -Vsource
      Range("Phi_n!b23:v23").Value = -Vsource
    Else
      Range("Phi_n!b3:v3").Value = 0
      Range("Phi_n!b23:v23").Value = 0
    End If
End Sub

Public Sub ResetPhi_p()
    Dim Vsource, Doping As Double
    Application.Calculation = xlAutomatic 'Restore recalc
    Vsource = Range("Excitation!g2")
    Doping = Range("Device!c6")
    If Doping > 0 Then
      Range("Phi_p!b3:v3").Value = 0
      Range("Phi_p!b23:v23").Value = 0
    Else
      Range("Phi_p!b3:v3").Value = Vsource
      Range("Phi_p!b23:v23").Value = Vsource
    End If
End Sub

Public Sub ResetQs()
    Application.Calculation = xlAutomatic 'Restore recalc
    Range("Gamma!b51:v71").Value = 0
    Range("Psi!b51:v71").Value = 0
End Sub

Public Sub ResetElectric()
  Call ResetPhi_n
  Call ResetPhi_p
  Call ResetQs
  Range("Excitation!b5") = RMSError()
  Range("Excitation!b8") = 0
End Sub

Public Sub UpdateQs()
    Dim Doping, Dp, Dn, Dmin, Damb, Q_LLI, Factor As Double
    Photogen = Range("Photogeneration!b15:v35")
    Recomb = Range("Recombination!b19:v39")
    Qgamma = Range("Gamma!b74:v94")  'Note this is the matrix that includes photogeneration
    Qpsi = Range("Psi!b51:v71")
    gamma = Range("Gamma!b216:v236")
    Doping = Range("Device!c6")
    Dp = Range("Device!f6")
    Dn = Range("Device!f8")
    Damb = 2 * Dn * Dp / (Dn + Dp)
    If Doping > 0 Then Dmin = Dn Else Dmin = Dp
    For i = 1 To 21
      For j = 1 To 21
        Factor = Photogen(i, j) - Recomb(i, j)
        Q_LLI = Photogen(i, j)
        Qpsi(i, j) = Factor / (1 + 2 * gamma(i, j)) * (Dn - Dp) / (Dn * Dp) * Dmin
        Qgamma(i, j) = Factor * Dmin / Damb + gamma(i, j)   'Qpsi term is added in the following line
        If Doping > 0 Then Qgamma(i, j) = Qgamma(i, j) - 0.5 * Qpsi(i, j) Else Qgamma(i, j) = Qgamma(i, j) + 0.5 * Qpsi(i, j)
        Qgamma(i, j) = Qgamma(i, j) - Q_LLI  'LLI is already accounted for in solver
        Next j
    Next i
    Application.Calculation = xlManual 'Delay recalc until both Q's are set
    Range("Psi!b51:v71") = Qpsi
    Range("Gamma!b51:v71") = Qgamma   'Note this is the matrix that does not include photogeneration
    Application.Calculation = xlAutomatic 'Restore recalc
End Sub

Public Sub IdentifyContacts(TopContact_n, BotContact_n, TopContact_p, BotContact_p)
    TopRho = Range("Device!b11:w11")      '22 values
    BotRho = Range("Device!b12:w12")      '22 values
    TopContact = Range("Device!b17:w17")  '22 values
    BotContact = Range("Device!b18:w18")  '22 values
    Doping = Range("Device!c6").Value
For i = 1 To 21              'Note these return values between -2 and +2 depending on contact arrangement
    If Doping > 0 Then
      If (TopRho(1, i) < 0 And TopContact(1, i) <> 0) Or (TopRho(1, i + 1) < 0 And TopContact(1, i + 1) <> 0) Then TopContact_n(i) = TopContact(1, i) + TopContact(1, i + 1) Else TopContact_n(i) = 0
      If (BotRho(1, i) < 0 And BotContact(1, i) <> 0) Or (BotRho(1, i + 1) < 0 And BotContact(1, i + 1) <> 0) Then BotContact_n(i) = BotContact(1, i) + BotContact(1, i + 1) Else BotContact_n(i) = 0
      If (TopRho(1, i) >= 0 And TopContact(1, i) <> 0) Or (TopRho(1, i + 1) >= 0 And TopContact(1, i + 1) <> 0) Then TopContact_p(i) = TopContact(1, i) + TopContact(1, i + 1) Else TopContact_p(i) = 0
      If (BotRho(1, i) >= 0 And BotContact(1, i) <> 0) Or (BotRho(1, i + 1) >= 0 And BotContact(1, i + 1) <> 0) Then BotContact_p(i) = BotContact(1, i) + BotContact(1, i + 1) Else BotContact_p(i) = 0
    Else
      If (TopRho(1, i) <= 0 And TopContact(1, i) <> 0) Or (TopRho(1, i + 1) <= 0 And TopContact(1, i + 1) <> 0) Then TopContact_n(i) = TopContact(1, i) + TopContact(1, i + 1) Else TopContact_n(i) = 0
      If (BotRho(1, i) <= 0 And BotContact(1, i) <> 0) Or (BotRho(1, i + 1) <= 0 And BotContact(1, i + 1) <> 0) Then BotContact_n(i) = BotContact(1, i) + BotContact(1, i + 1) Else BotContact_n(i) = 0
      If (TopRho(1, i) > 0 And TopContact(1, i) <> 0) Or (TopRho(1, i + 1) > 0 And TopContact(1, i + 1) <> 0) Then TopContact_p(i) = TopContact(1, i) + TopContact(1, i + 1) Else TopContact_p(i) = 0
      If (BotRho(1, i) > 0 And BotContact(1, i) <> 0) Or (BotRho(1, i + 1) > 0 And BotContact(1, i + 1) <> 0) Then BotContact_p(i) = BotContact(1, i) + BotContact(1, i + 1) Else BotContact_p(i) = 0
    End If
    Next i
End Sub

Function RMSError() As Double
  Dim TopContact_n(21), BotContact_n(21), TopContact_p(21), BotContact_p(21) As Integer
  Dim Error As Double
  Dim count As Integer
  Call IdentifyContacts(TopContact_n, BotContact_n, TopContact_p, BotContact_p)
      
  TopError_n = Range("Phi_n!b26:v26")
  BotError_n = Range("Phi_n!b27:v27")
  TopError_p = Range("Phi_p!b26:v26")
  BotError_p = Range("Phi_p!b27:v27")
  Error = 0
  count = 0
  For i = 1 To 21
    If TopContact_n(i) = 0 Then
      Error = Error + TopError_n(1, i) ^ 2
      count = count + 1
      End If
    If BotContact_n(i) = 0 Then
      Error = Error + BotError_n(1, i) ^ 2
      count = count + 1
      End If
    If TopContact_p(i) = 0 Then
      Error = Error + TopError_p(1, i) ^ 2
      count = count + 1
      End If
    If BotContact_p(i) = 0 Then
      Error = Error + BotError_p(1, i) ^ 2
      count = count + 1
      End If
    Next i
  If count = 0 Then RMSError = 0 Else RMSError = Sqr(Error / count)
   
End Function
    
Public Sub SolveElectric()
  Dim Max As Integer
  Dim Convergence, Error As Double
  Max = Range("Excitation!b7")
  Convergence = Range("Excitation!b6")
  
  Error = RMSError()
  
  Call UpdateQs
  For m = 1 To Max
    Range("Excitation!b8") = m
    Call Jacobian
    Call UpdateQs
    Error = RMSError()
    Range("Excitation!b5") = Error
    If Error < Convergence Then Exit For
    Next m
  If Error < Convergence Then Range("Excitation!b8").Value = "Done" Else Range("Excitation!b8").Value = "Not Done"

End Sub

Sub SolveButton()
Call SolveGeneration
Call SolveElectric
End Sub

Sub DiffusivityModel()
'This calculates diffusivities using the PC1D model for silicon at 25C
Ndop = Range("Device!c6").Value

'Calculate electron diffusivity
Max = 36.4
If Ndop < 0 Then
  Min = 1.5
  Nref = 9.64E+16
  alpha = 0.664
Else
  Min = 4.1
  Nref = 5.6E+16
  alpha = 0.647
End If
Dn = Min * 1.004 + (Max - Min) * 1.015 / (1 + (Abs(Ndop) / Nref / 0.985) ^ (alpha * 1.001))
Range("Device!f8").Value = Dn

'Calculate hole diffusivity
Max = 12.1
If Ndop > 0 Then
  Min = 1
  Nref = 2.82E+17
  alpha = 0.642
Else
  Min = 4
  Nref = 1E+17
  alpha = 0.9
End If
Dp = Min * 1.004 + (Max - Min) * 1.014 / (1 + (Abs(Ndop) / Nref / 0.985) ^ (alpha * 1.001))
Range("Device!f6").Value = Dp

End Sub
