Attribute VB_Name = "Module7"
Sub Jacobian()
    Dim Delta, Doping, Vsource As Double
    Delta = Range("Jacobian!s1")
    Doping = Range("Device!c6")
    Vsource = Range("Excitation!g2")
    TopPhi_n = Range("Phi_n!b3:v3")
    BotPhi_n = Range("Phi_n!b23:v23")
    TopFi_n = Range("Phi_n!b26:v26")
    BotFi_n = Range("Phi_n!b27:v27")
    TopPhi_p = Range("Phi_p!b3:v3")
    BotPhi_p = Range("Phi_p!b23:v23")
    TopFi_p = Range("Phi_p!b26:v26")
    BotFi_p = Range("Phi_p!b27:v27")
    Recomb = Range("Recombination!b14:v15")
    nShunt = Range("Phi_n!b38:v39")
    pShunt = Range("Phi_p!b38:v39")
    nShuntContact = Range("Phi_n!b35:v36")
    pShuntContact = Range("Phi_p!b35:v36")
    BranchConductance = Range("Device!m20")
    A = Range("Jacobian!b5:cg90")
    Dim SavePhi, SaveRecomb, SaveShunt As Double
    
    Dim TopContact_n(21), BotContact_n(21), TopContact_p(21), BotContact_p(21) As Integer
    Call IdentifyContacts(TopContact_n, BotContact_n, TopContact_p, BotContact_p)
   
    For i = 1 To 21          'Adjust applied potential at contacts
        If TopContact_n(i) > 0 Then If Doping > 0 Then TopPhi_n(1, i) = -Vsource Else TopPhi_n(1, i) = 0
        If BotContact_n(i) > 0 Then If Doping > 0 Then BotPhi_n(1, i) = -Vsource Else BotPhi_n(1, i) = 0
        If TopContact_p(i) > 0 Then If Doping < 0 Then TopPhi_p(1, i) = Vsource Else TopPhi_p(1, i) = 0
        If BotContact_p(i) > 0 Then If Doping < 0 Then BotPhi_p(1, i) = Vsource Else BotPhi_p(1, i) = 0
'Added to allow modeling of lateral series cells
        If TopContact_n(i) < 0 Then If Doping > 0 Then TopPhi_n(1, i) = 0 Else TopPhi_n(1, i) = -Vsource
        If BotContact_n(i) < 0 Then If Doping > 0 Then BotPhi_n(1, i) = 0 Else BotPhi_n(1, i) = -Vsource
        If TopContact_p(i) < 0 Then If Doping < 0 Then TopPhi_p(1, i) = 0 Else TopPhi_p(1, i) = Vsource
        If BotContact_p(i) < 0 Then If Doping < 0 Then BotPhi_p(1, i) = 0 Else BotPhi_p(1, i) = Vsource
        Next i
        
    For i = 1 To 21                'Store unperturbed Phi and Fi
      A(1, i) = TopPhi_n(1, i)
      If TopContact_n(i) <> 0 Then A(2, i) = 0 Else A(2, i) = TopFi_n(1, i)
      A(1, 21 + i) = BotPhi_n(1, i)
      If BotContact_n(i) <> 0 Then A(2, 21 + i) = 0 Else A(2, 21 + i) = BotFi_n(1, i)
      A(1, 42 + i) = TopPhi_p(1, i)
      If TopContact_p(i) <> 0 Then A(2, 42 + i) = 0 Else A(2, 42 + i) = TopFi_p(1, i)
      A(1, 63 + i) = BotPhi_p(1, i)
      If BotContact_p(i) <> 0 Then A(2, 63 + i) = 0 Else A(2, 63 + i) = BotFi_p(1, i)
      Next i
    
    For j = 1 To 21                 'Perturb each top Phi_n
      SavePhi = TopPhi_n(1, j)
      SaveRecomb = Recomb(1, j)
      TopPhi_n(1, j) = TopPhi_n(1, j) - Delta
      Recomb(1, j) = CalcTopRecomb(j, TopPhi_n, TopPhi_p)
      SaveShunt = nShunt(1, j)
      If nShuntContact(1, j) > 0 Then nShunt(1, j) = nShunt(1, j) + Delta * BranchConductance
      Call CalculateError(TopPhi_n, BotPhi_n, TopPhi_p, BotPhi_p, TopFi_n, BotFi_n, TopFi_p, BotFi_p, Recomb, nShunt, pShunt)
      For i = 1 To 21
        If TopContact_n(i) <> 0 Then
          If i = j Then A(j + 2, i) = Delta Else A(j + 2, i) = 0
          Else: A(j + 2, i) = TopFi_n(1, i)
          End If
        If BotContact_n(i) <> 0 Then A(j + 2, i + 21) = 0 Else A(j + 2, i + 21) = BotFi_n(1, i)
        If TopContact_p(i) <> 0 Then A(j + 2, i + 42) = 0 Else A(j + 2, i + 42) = TopFi_p(1, i)
        If BotContact_p(i) <> 0 Then A(j + 2, i + 63) = 0 Else A(j + 2, i + 63) = BotFi_p(1, i)
        Next i
      TopPhi_n(1, j) = SavePhi
      Recomb(1, j) = SaveRecomb
      nShunt(1, j) = SaveShunt
      Next j
    
    For j = 1 To 21                 'Perturb each bot Phi_n
      SavePhi = BotPhi_n(1, j)
      SaveRecomb = Recomb(2, j)
      BotPhi_n(1, j) = BotPhi_n(1, j) - Delta
      Recomb(2, j) = CalcBotRecomb(j, BotPhi_n, BotPhi_p)
      SaveShunt = nShunt(2, j)
      If nShuntContact(2, j) > 0 Then nShunt(2, j) = nShunt(2, j) + Delta * BranchConductance
      Call CalculateError(TopPhi_n, BotPhi_n, TopPhi_p, BotPhi_p, TopFi_n, BotFi_n, TopFi_p, BotFi_p, Recomb, nShunt, pShunt)
      For i = 1 To 21
        If TopContact_n(i) <> 0 Then A(j + 23, i) = 0 Else A(j + 23, i) = TopFi_n(1, i)
        If BotContact_n(i) <> 0 Then
          If i = j Then A(j + 23, i + 21) = Delta Else A(j + 23, i + 21) = 0
          Else: A(j + 23, i + 21) = BotFi_n(1, i)
          End If
        If TopContact_p(i) <> 0 Then A(j + 23, i + 42) = 0 Else A(j + 23, i + 42) = TopFi_p(1, i)
        If BotContact_p(i) <> 0 Then A(j + 23, i + 63) = 0 Else A(j + 23, i + 63) = BotFi_p(1, i)
        Next i
      BotPhi_n(1, j) = SavePhi
      Recomb(2, j) = SaveRecomb
      nShunt(2, j) = SaveShunt
      Next j
    
    For j = 1 To 21                 'Perturb each top Phi_p
      SavePhi = TopPhi_p(1, j)
      SaveRecomb = Recomb(1, j)
      TopPhi_p(1, j) = TopPhi_p(1, j) + Delta
      Recomb(1, j) = CalcTopRecomb(j, TopPhi_n, TopPhi_p)
      SaveShunt = pShunt(1, j)
      If pShuntContact(1, j) > 0 Then pShunt(1, j) = pShunt(1, j) - Delta * BranchConductance
      Call CalculateError(TopPhi_n, BotPhi_n, TopPhi_p, BotPhi_p, TopFi_n, BotFi_n, TopFi_p, BotFi_p, Recomb, nShunt, pShunt)
      For i = 1 To 21
        If TopContact_n(i) <> 0 Then A(j + 44, i) = 0 Else A(j + 44, i) = TopFi_n(1, i)
        If BotContact_n(i) <> 0 Then A(j + 44, i + 21) = 0 Else A(j + 44, i + 21) = BotFi_n(1, i)
        If TopContact_p(i) <> 0 Then
          If i = j Then A(j + 44, i + 42) = Delta Else A(j + 44, i + 42) = 0
          Else: A(j + 44, i + 42) = TopFi_p(1, i)
          End If
        If BotContact_p(i) <> 0 Then A(j + 44, i + 63) = 0 Else A(j + 44, i + 63) = BotFi_p(1, i)
        Next i
      TopPhi_p(1, j) = SavePhi
      Recomb(1, j) = SaveRecomb
      pShunt(1, j) = SaveShunt
      Next j
    
    For j = 1 To 21                 'Perturb each bot Phi_p
      SavePhi = BotPhi_p(1, j)
      SaveRecomb = Recomb(2, j)
      BotPhi_p(1, j) = BotPhi_p(1, j) + Delta
      Recomb(2, j) = CalcBotRecomb(j, BotPhi_n, BotPhi_p)
      SaveShunt = pShunt(2, j)
      If pShuntContact(2, j) > 0 Then pShunt(2, j) = pShunt(2, j) - Delta * BranchConductance
      Call CalculateError(TopPhi_n, BotPhi_n, TopPhi_p, BotPhi_p, TopFi_n, BotFi_n, TopFi_p, BotFi_p, Recomb, nShunt, pShunt)
      For i = 1 To 21
        If TopContact_n(i) <> 0 Then A(j + 65, i) = 0 Else A(j + 65, i) = TopFi_n(1, i)
        If BotContact_n(i) <> 0 Then A(j + 65, i + 21) = 0 Else A(j + 65, i + 21) = BotFi_n(1, i)
        If TopContact_p(i) <> 0 Then A(j + 65, i + 42) = 0 Else A(j + 65, i + 42) = TopFi_p(1, i)
        If BotContact_p(i) <> 0 Then
          If i = j Then A(j + 65, i + 63) = Delta Else A(j + 65, i + 63) = 0
          Else: A(j + 65, i + 63) = BotFi_p(1, i)
          End If
        Next i
      BotPhi_p(1, j) = SavePhi
      Recomb(2, j) = SaveRecomb
      pShunt(2, j) = SaveShunt
      Next j
    
    Range("Jacobian!b5:cg90") = A   'Initiates Jacobian calculation with new set of Phi's
    TopPhi_n = Range("Jacobian!b355:v355")
    BotPhi_n = Range("Jacobian!w355:aq355")
    TopPhi_p = Range("Jacobian!ar355:bl355")
    BotPhi_p = Range("Jacobian!bm355:cg355")
    Application.Calculation = xlManual    'Turn off recalc until all new Phis are set
    Range("Phi_n!b3:v3") = TopPhi_n
    Range("Phi_n!b23:v23") = BotPhi_n
    Range("Phi_p!b3:v3") = TopPhi_p
    Range("Phi_p!b23:v23") = BotPhi_p
    Application.Calculation = xlAutomatic 'Restore recalc

End Sub

Sub CalculateError(TopPhi_n, BotPhi_n, TopPhi_p, BotPhi_p, TopFi_n, BotFi_n, TopFi_p, BotFi_p, Recomb, nShunt, pShunt)
'   First four arguments are inputs, Row format
'   Next four arguments are outputs, Row format
'   Remaining three arguments are inputs, Row format 1=Top 2=Bot
'   Need to be watchful about row vs column format for MMult function to work properly

'Column array format structured from Psi Worksheet, but does not use those values
ColumnArray21 = Range("Psi!b28:b48")
'Real space surface arrays
TopPsi = ColumnArray21
BotPsi = ColumnArray21
HGradTopPsi = ColumnArray21
HGradBotPsi = ColumnArray21
TopGamma = ColumnArray21
BotGamma = ColumnArray21
HGradTopGamma = ColumnArray21
HGradBotGamma = ColumnArray21
TopElec = ColumnArray21
BotElec = ColumnArray21
TopHole = ColumnArray21
BotHole = ColumnArray21
'Fourier space surface arrays
FTopPsi = ColumnArray21
FBotPsi = ColumnArray21
FHGradTopPsi = ColumnArray21
FHGradBotPsi = ColumnArray21
FTopGamma = ColumnArray21
FBotGamma = ColumnArray21
FHGradTopGamma = ColumnArray21
FHGradBotGamma = ColumnArray21
'Constant arrays (copied from Worksheets)
RealtoFourier = Range("Transform!b29:v49")
FouriertoReal = Range("Transform!b6:v26")
Sum0CmnGamma = Range("Gamma!x192:x212")  'Column
Sum1CmnGamma = Range("Gamma!y192:y212")  'Column
knHgamma = Range("Gamma!ab192:ab212") 'Column
Sum0CmnPsi = Range("Psi!x169:x189")  'Column
Sum1CmnPsi = Range("Psi!y169:y189")  'Column
knhPsi = Range("Psi!ab169:ab189")     'Column
nTopRho_Left = Range("Phi_n!b29:v29")  'Row
nBotRho_Left = Range("Phi_n!b30:v30") 'Row
nTopRho_Right = Range("Phi_n!b32:v32")  'Row
nBotRho_Right = Range("Phi_n!b33:v33") 'Row
pTopRho_Left = Range("Phi_p!b29:v29")  'Row
pBotRho_Left = Range("Phi_p!b30:v30") 'Row
pTopRho_Right = Range("Phi_p!b32:v32")  'Row
pBotRho_Right = Range("Phi_p!b33:v33") 'Row
'Constant values (copied from Worksheets)
Dim Doping, Vt, PsiEq, ni, ni_n, N_2ni, Io_n, Io_p, d00, H2, L2 As Double
Doping = Range("Device!c6")
Vt = Range("Device!o6")
PsiEq = Range("Psi!k1")
ni = Range("Device!s8")
ni_n = ni / Abs(Doping)
N_2ni = Range("Psi!s1")
Io_n = Range("Phi_n!s1")
Io_p = Range("Phi_p!s1")
d00 = Range("Psi!b97")
H2 = Range("Device!f3") ^ 2
L2 = Range("Device!i8") ^ 2
c00 = d00 * H2 / L2
'Values used locally
Dim an, bn, knH As Double
Dim Sinh_knH, Tanh_knH As Double
Dim i, n As Integer

For i = 1 To 21  'Real Space increments in x
  If Doping > 0 Then
    TopPsi(i, 1) = TopPhi_p(1, i) - Vt * Log(N_2ni + Sqr(N_2ni ^ 2 + Exp((TopPhi_p(1, i) - TopPhi_n(1, i)) / Vt))) - PsiEq
    BotPsi(i, 1) = BotPhi_p(1, i) - Vt * Log(N_2ni + Sqr(N_2ni ^ 2 + Exp((BotPhi_p(1, i) - BotPhi_n(1, i)) / Vt))) - PsiEq
    TopGamma(i, 1) = ni_n * Exp(PsiEq / Vt) * (Exp((TopPsi(i, 1) - TopPhi_n(1, i)) / Vt) - 1)
    BotGamma(i, 1) = ni_n * Exp(PsiEq / Vt) * (Exp((BotPsi(i, 1) - BotPhi_n(1, i)) / Vt) - 1)
  Else
    TopPsi(i, 1) = TopPhi_n(1, i) + Vt * Log(N_2ni + Sqr(N_2ni ^ 2 + Exp((TopPhi_p(1, i) - TopPhi_n(1, i)) / Vt))) - PsiEq
    BotPsi(i, 1) = BotPhi_n(1, i) + Vt * Log(N_2ni + Sqr(N_2ni ^ 2 + Exp((BotPhi_p(1, i) - BotPhi_n(1, i)) / Vt))) - PsiEq
    TopGamma(i, 1) = ni_n * Exp(-PsiEq / Vt) * (Exp(-(TopPsi(i, 1) - TopPhi_p(1, i)) / Vt) - 1)
    BotGamma(i, 1) = ni_n * Exp(-PsiEq / Vt) * (Exp(-(BotPsi(i, 1) - BotPhi_p(1, i)) / Vt) - 1)
  End If
  TopElec(i, 1) = ni_n * Exp((PsiEq + TopPsi(i, 1) - TopPhi_n(1, i)) / Vt)
  BotElec(i, 1) = ni_n * Exp((PsiEq + BotPsi(i, 1) - BotPhi_n(1, i)) / Vt)
  TopHole(i, 1) = ni_n * Exp(-(PsiEq + TopPsi(i, 1) - TopPhi_p(1, i)) / Vt)
  BotHole(i, 1) = ni_n * Exp(-(PsiEq + BotPsi(i, 1) - BotPhi_p(1, i)) / Vt)
  Next i
FTopGamma = Application.WorksheetFunction.MMult(RealtoFourier, TopGamma)
FBotGamma = Application.WorksheetFunction.MMult(RealtoFourier, BotGamma)
FTopPsi = Application.WorksheetFunction.MMult(RealtoFourier, TopPsi)
FBotPsi = Application.WorksheetFunction.MMult(RealtoFourier, BotPsi)

For n = 0 To 20  'Harmonic increments in x
  'Gamma
  an = FBotGamma(n + 1, 1) - Sum1CmnGamma(n + 1, 1)
  bn = FTopGamma(n + 1, 1) - Sum0CmnGamma(n + 1, 1)
  knH = knHgamma(n + 1, 1)
  Sinh_knH = Sinh(knH)
  Tanh_knH = Sinh_knH / Cosh(knH)
  FHGradTopGamma(n + 1, 1) = knH * (an / Sinh_knH - bn / Tanh_knH)
  FHGradBotGamma(n + 1, 1) = knH * (an / Tanh_knH - bn / Sinh_knH)
  'Psi
  an = FBotPsi(n + 1, 1) / Vt - Sum1CmnPsi(n + 1, 1)
  bn = FTopPsi(n + 1, 1) / Vt - Sum0CmnPsi(n + 1, 1)
  knH = knhPsi(n + 1, 1)
  Sinh_knH = Sinh(knH)
  Tanh_knH = Sinh_knH / Cosh(knH)
  If n = 0 Then
    FHGradTopPsi(n + 1, 1) = Vt * (an - bn + 0.5 * c00)
    FHGradBotPsi(n + 1, 1) = Vt * (an - bn - 0.5 * c00)
  Else
    FHGradTopPsi(n + 1, 1) = Vt * knH * (an / Sinh_knH - bn / Tanh_knH)
    FHGradBotPsi(n + 1, 1) = Vt * knH * (an / Tanh_knH - bn / Sinh_knH)
  End If
  Next n
HGradTopGamma = Application.WorksheetFunction.MMult(FouriertoReal, FHGradTopGamma)
HGradBotGamma = Application.WorksheetFunction.MMult(FouriertoReal, FHGradBotGamma)
HGradTopPsi = Application.WorksheetFunction.MMult(FouriertoReal, FHGradTopPsi)
HGradBotPsi = Application.WorksheetFunction.MMult(FouriertoReal, FHGradBotPsi)


i = 1       'Special treatment of sheet current at left edge due to symmetry boundary
  TopFi_n(1, i) = Io_n * (HGradTopGamma(i, 1) - TopElec(i, 1) * HGradTopPsi(i, 1) / Vt) _
        + 2 * (TopPhi_n(1, i) - TopPhi_n(1, i + 1)) * nTopRho_Right(1, i) _
        - Recomb(1, i) - nShunt(1, i)
  BotFi_n(1, i) = Io_n * (-HGradBotGamma(i, 1) + BotElec(i, 1) * HGradBotPsi(i, 1) / Vt) _
        + 2 * (BotPhi_n(1, i) - BotPhi_n(1, i + 1)) * nBotRho_Right(1, i) _
        - Recomb(2, i) - nShunt(2, i)
  TopFi_p(1, i) = Io_p * (-HGradTopGamma(i, 1) - TopHole(i, 1) * HGradTopPsi(i, 1) / Vt) _
        + 2 * (TopPhi_p(1, i) - TopPhi_p(1, i + 1)) * pTopRho_Right(1, i) _
        + Recomb(1, i) - pShunt(1, i)
  BotFi_p(1, i) = Io_p * (HGradBotGamma(i, 1) + BotHole(i, 1) * HGradBotPsi(i, 1) / Vt) _
        + 2 * (BotPhi_p(1, i) - BotPhi_p(1, i + 1)) * pBotRho_Right(1, i) _
        + Recomb(2, i) - pShunt(2, i)
For i = 2 To 20          'Has no surface recombination or sheet conductance yet!
  TopFi_n(1, i) = Io_n * (HGradTopGamma(i, 1) - TopElec(i, 1) * HGradTopPsi(i, 1) / Vt) _
        + (TopPhi_n(1, i) - TopPhi_n(1, i + 1)) * nTopRho_Right(1, i) + (TopPhi_n(1, i) - TopPhi_n(1, i - 1)) * nTopRho_Left(1, i) _
        - Recomb(1, i) - nShunt(1, i)
  BotFi_n(1, i) = Io_n * (-HGradBotGamma(i, 1) + BotElec(i, 1) * HGradBotPsi(i, 1) / Vt) _
        + (BotPhi_n(1, i) - BotPhi_n(1, i + 1)) * nBotRho_Right(1, i) + (BotPhi_n(1, i) - BotPhi_n(1, i - 1)) * nBotRho_Left(1, i) _
        - Recomb(2, i) - nShunt(2, i)
  TopFi_p(1, i) = Io_p * (-HGradTopGamma(i, 1) - TopHole(i, 1) * HGradTopPsi(i, 1) / Vt) _
        + (TopPhi_p(1, i) - TopPhi_p(1, i + 1)) * pTopRho_Right(1, i) + (TopPhi_p(1, i) - TopPhi_p(1, i - 1)) * pTopRho_Left(1, i) _
        + Recomb(1, i) - pShunt(1, i)
  BotFi_p(1, i) = Io_p * (HGradBotGamma(i, 1) + BotHole(i, 1) * HGradBotPsi(i, 1) / Vt) _
        + (BotPhi_p(1, i) - BotPhi_p(1, i + 1)) * pBotRho_Right(1, i) + (BotPhi_p(1, i) - BotPhi_p(1, i - 1)) * pBotRho_Left(1, i) _
        + Recomb(2, i) - pShunt(2, i)
  Next i
i = 21      'Special treatment of sheet current at right edge due to symmetry boundary
  TopFi_n(1, i) = Io_n * (HGradTopGamma(i, 1) - TopElec(i, 1) * HGradTopPsi(i, 1) / Vt) _
        + 2 * (TopPhi_n(1, i) - TopPhi_n(1, i - 1)) * nTopRho_Left(1, i) _
        - Recomb(1, i) - nShunt(1, i)
  BotFi_n(1, i) = Io_n * (-HGradBotGamma(i, 1) + BotElec(i, 1) * HGradBotPsi(i, 1) / Vt) _
        + 2 * (BotPhi_n(1, i) - BotPhi_n(1, i - 1)) * nBotRho_Left(1, i) _
        - Recomb(2, i) - nShunt(2, i)
  TopFi_p(1, i) = Io_p * (-HGradTopGamma(i, 1) - TopHole(i, 1) * HGradTopPsi(i, 1) / Vt) _
        + 2 * (TopPhi_p(1, i) - TopPhi_p(1, i - 1)) * pTopRho_Left(1, i) _
        + Recomb(1, i) - pShunt(1, i)
  BotFi_p(1, i) = Io_p * (HGradBotGamma(i, 1) + BotHole(i, 1) * HGradBotPsi(i, 1) / Vt) _
        + 2 * (BotPhi_p(1, i) - BotPhi_p(1, i - 1)) * pBotRho_Left(1, i) _
        + Recomb(2, i) - pShunt(2, i)

End Sub

Function CalcTopRecomb(i, Phi_n, Phi_p) As Double
Dim pn_ni2, Vt, deltax, deltaz As Double

' Constant row arrays
Jo1 = Range("Recombination!b3:u3")  '20 Top values
Jo2 = Range("Recombination!b7:u7")  '20 Top values
' Constant real values
Vt = Range("Device!o6")
deltax = Range("Device!c2")
deltaz = Range("Device!i2")

pn_ni2 = Exp((Phi_p(1, i) - Phi_n(1, i)) / Vt)

If i = 1 Then
    CalcTopRecomb = (Jo1(1, i) * (pn_ni2 - 1) + Jo2(1, i) * (Sqr(pn_ni2) - 1)) * deltax * deltaz
Else
  If i = 21 Then
    CalcTopRecomb = (Jo1(1, i - 1) * (pn_ni2 - 1) + Jo2(1, i - 1) * (Sqr(pn_ni2) - 1)) * deltax * deltaz
  Else
    CalcTopRecomb = (0.5 * (Jo1(1, i - 1) + Jo1(1, i)) * (pn_ni2 - 1) + 0.5 * (Jo2(1, i - 1) + Jo2(1, i)) * (Sqr(pn_ni2) - 1)) * deltax * deltaz
  End If
End If

End Function

Function CalcBotRecomb(i, Phi_n, Phi_p) As Double
Dim pn_ni2, Vt, deltax, deltaz As Double

' Constant row arrays
Jo1 = Range("Recombination!b5:u5")  '20 Bot values
Jo2 = Range("Recombination!b9:u9")  '20 Bot values
' Constant real values
Vt = Range("Device!o6")
deltax = Range("Device!c2")
deltaz = Range("Device!i2")

pn_ni2 = Exp((Phi_p(1, i) - Phi_n(1, i)) / Vt)

If i = 1 Then
    CalcBotRecomb = (Jo1(1, i) * (pn_ni2 - 1) + Jo2(1, i) * (Sqr(pn_ni2) - 1)) * deltax * deltaz
Else
  If i = 21 Then
    CalcBotRecomb = (Jo1(1, i - 1) * (pn_ni2 - 1) + Jo2(1, i - 1) * (Sqr(pn_ni2) - 1)) * deltax * deltaz
  Else
    CalcBotRecomb = (0.5 * (Jo1(1, i - 1) + Jo1(1, i)) * (pn_ni2 - 1) + 0.5 * (Jo2(1, i - 1) + Jo2(1, i)) * (Sqr(pn_ni2) - 1)) * deltax * deltaz
  End If
End If

End Function

Function Sinh(x As Double) As Double
Sinh = 0.5 * (Exp(x) - Exp(-x))
End Function

Function Cosh(x As Double) As Double
Cosh = 0.5 * (Exp(x) + Exp(-x))
End Function
