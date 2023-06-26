Attribute VB_Name = "Module4"
Public Const Num As Integer = 20   'Constant number of columns and rows in the solution region
Public Const MaxLambda As Integer = 60
Public Const SmallDelta As Double = 0.001
Public Const MaxPasses As Integer = 100


Sub ResetGeneration()
  Range("Photogeneration!b15:v35").ClearContents
  Range("Photogeneration!a297:x356").ClearContents
  Range("Photogeneration!a361:x420").ClearContents
  Range("Photogeneration!a425:x484").ClearContents
  Range("Photogeneration!a489:x548").ClearContents
  Range("Excitation!b8") = "PhotoGen"
  Call UpdateScreen
End Sub

Sub SolveGeneration()
  Dim i, j, k, n, lambda, iteration, SourceIndexRight, SourceIndexLeft As Integer
  Dim TopSpec(Num), TopDiff(Num), BotSpec(Num), BotDiff(Num) As Double  'These are light-trapped fluxes arriving/leaving named surface
  Dim SpecTop0(Num), SpecBot0(Num), DiffTop0(Num), DiffBot0(Num) As Double  'These are fluxes prior to any light trapping
  Dim SpecTopFront(Num), SpecTopRear(Num), SpecBotFront(Num), SpecBotRear(Num) As Double   'These are total fluxes leaving named surface
  Dim DiffTopFront(Num), DiffTopRear(Num), DiffBotFront(Num), DiffBotRear(Num) As Double   'Front, rear refer to illumination source
  
  Dim alpha, alphaH, PreFactor, Delta, Delta_y, VF, A, H, W, dx, r As Double
  Dim FrontIllumination, Mono As Boolean
  Dim x, gamma, gamma_prime      'Factors in Green's diffuse transmission expression
  Dim Absorb_calc, Absorb_sum, Diffuse, Specular As Double
  
  Call ResetGeneration

'Column array format structured from Psi Worksheet, but does not use those values
  ColumnArray21 = Range("Psi!b28:b48")
'Real space surface arrays
  SpecTopCol = ColumnArray21
  SpecBotCol = ColumnArray21
  DiffTopCol = ColumnArray21
  DiffBotCol = ColumnArray21
'Fourier space surface arrays
  FSpecTopCol = ColumnArray21
  FSpecBotCol = ColumnArray21
  FDiffTopCol = ColumnArray21
  FDiffBotCol = ColumnArray21
'Copy arrays from worksheet
  Range("Photogeneration!b15:v35").ClearContents
  PhotogenRate = Range("Photogeneration!b15:v35")
  SpecTopTable = Range("Photogeneration!a41:x100")  'lambda (nm), alpha (cm-1), flux (A/cm2), spectral trans, spatial data (A)
  DiffTopTable = Range("Photogeneration!a105:x164")
  SpecBotTable = Range("Photogeneration!a169:x228")
  DiffBotTable = Range("Photogeneration!a233:x292")
  Htop = Range("Photogeneration!b6:u6")
  RspecTop = Range("Photogeneration!b7:u7")
  RdiffTop = Range("Photogeneration!b8:u8")
  Hbot = Range("Photogeneration!b9:u9")
  RspecBot = Range("Photogeneration!b10:u10")
  RdiffBot = Range("Photogeneration!b11:u11")
  DiffArray = Range("Photogeneration!b15:v35")
  RealtoFourier = Range("Transform!b29:v49")
  FouriertoReal = Range("Transform!b6:v26")
'Get constants from worksheet for calculations
  W = Range("Device!C3").Value
  H = Range("Device!F3").Value
  q = Range("Device!O8").Value
  tau_N = Range("Photogeneration!L14").Value
  FrontIllumination = (Range("Photogeneration!E1").Value = "Front")
  Mono = (Range("Photogeneration!H1").Value = "Mono")
  Pi = WorksheetFunction.Pi

'Don't execute if no light
  If Range("Photogeneration!H1").Value = "None" Or Range("Photogeneration!N14") = 0 Then Exit Sub

A = H / W

If Mono Then
    StartLambda = 1: EndLambda = 1
Else
    StartLambda = 2: EndLambda = MaxLambda
End If

'Outer loop for each wavelength to be evaluated

For lambda = StartLambda To EndLambda
  alpha = SpecTopTable(lambda, 2)
  alphaH = alpha * H
  If alphaH * MaxPasses < SmallDelta Then Exit For
  x = 0.935 * Application.WorksheetFunction.Power(alphaH, 0.67)

'**** Initialize Specular Light ****
For i = 1 To Num
  SpecTop0(i) = SpecTopTable(lambda, i + 4)
  SpecBot0(i) = SpecBotTable(lambda, i + 4)
  SpecTopFront(i) = SpecTop0(i)
  SpecBotRear(i) = SpecBot0(i)
Next i

'**** Initialize Diffuse Light ****
For i = 1 To Num
  DiffTop0(i) = DiffTopTable(lambda, i + 4)
  DiffBot0(i) = DiffBotTable(lambda, i + 4)
  DiffTopFront(i) = DiffTop0(i)
  DiffBotRear(i) = DiffBot0(i)
Next i

For iteration = 1 To MaxPasses  'Maximum round-trip passes of the light. Most cases will exit this loop much sooner
  MaxSource = 0
  MaxDelta = 0
    
If FrontIllumination Then

'Calculate light trapping augmentation due to front illumination
   
    'Calculate front specular flux reaching Bot surface at each element
    For i = 1 To Num
      If SpecTopFront(i) > MaxSource Then MaxSource = SpecTopFront(i)
      If alphaH < 10 Then  'Won't underflow exponential
        BotSpec(i) = SpecTopFront(i) * Exp(-alphaH)
      Else: BotSpec(i) = 0
      End If
    Next i
   
   For i = 1 To Num
     If DiffTopFront(i) > MaxSource Then MaxSource = DiffTopFront(i)
     Next i
     
   'Calculate front diffuse flux reaching Bot surface at each element
   DiffTopCol(1, 1) = DiffTopFront(1)     'Leftmost node
   For i = 2 To Num
     DiffTopCol(i, 1) = 0.5 * (DiffTopFront(i - 1) + DiffTopFront(i))
     Next i
   DiffTopCol(21, 1) = DiffTopFront(20)   'Rightmost node
   FDiffTopCol = Application.WorksheetFunction.MMult(RealtoFourier, DiffTopCol)
   For n = 0 To Num
     gamma = (2 + x) / (1 + x + 0.4 * Pi * n * A)
     If gamma * alphaH < 10 And n * A < 10 Then
       FDiffBotCol(n + 1, 1) = FDiffTopCol(n + 1, 1) * (2 / (1 + Exp(0.91 * Pi * n * A))) * Exp(-gamma * alphaH)
     Else: FDiffBotCol(n + 1, 1) = 0
     End If
     Next n
   DiffBotCol = Application.WorksheetFunction.MMult(FouriertoReal, FDiffBotCol)
   For i = 1 To Num
     BotDiff(i) = 0.5 * (DiffBotCol(i, 1) + DiffBotCol(i + 1, 1))
     Next i
        
   'Convert fluxes reaching Bot surface to spectral and diffuse flux reflected from Bot surface
   gamma = (2 + x) / (1 + x)
   For i = 1 To Num
     Specular = BotSpec(i) * RspecBot(1, i) * (1 - Hbot(1, i)) + BotDiff(i) * RspecBot(1, i) * (2 - gamma) * (1 - Hbot(1, i))
     Diffuse = BotSpec(i) * RdiffBot(1, i) * Hbot(1, i) + BotDiff(i) * RdiffBot(1, i) * ((gamma - 1) + Hbot(1, i) * (2 - gamma))
     BotSpec(i) = Specular
     BotDiff(i) = Diffuse
   Next i
   
   'Calculate reflected specular flux reaching Top surface at each element
   For i = 1 To Num
     If alphaH < 10 Then  'Won't underflow exponential
       TopSpec(i) = BotSpec(i) * Exp(-alphaH)
     Else: TopSpec(i) = 0
     End If
   Next i
   
   'Calculate reflected diffuse flux reaching Top surface at each element
   DiffBotCol(1, 1) = BotDiff(1)     'Leftmost node
   For i = 2 To Num
     DiffBotCol(i, 1) = 0.5 * (BotDiff(i - 1) + BotDiff(i))
     Next i
   DiffBotCol(21, 1) = BotDiff(20)   'Rightmost node
   FDiffBotCol = Application.WorksheetFunction.MMult(RealtoFourier, DiffBotCol)
   For n = 0 To Num
     gamma = (2 + x) / (1 + x + 0.4 * Pi * n * A)
     If gamma * alphaH < 10 And n * A < 10 Then
       FDiffTopCol(n + 1, 1) = FDiffBotCol(n + 1, 1) * (2 / (1 + Exp(0.91 * Pi * n * A))) * Exp(-gamma * alphaH)
     Else: FDiffTopCol(n + 1, 1) = 0
     End If
     Next n
   DiffTopCol = Application.WorksheetFunction.MMult(FouriertoReal, FDiffTopCol)
   For i = 1 To Num
     TopDiff(i) = 0.5 * (DiffTopCol(i, 1) + DiffTopCol(i + 1, 1))
     Next i
   
   'Convert fluxes reaching Top surface to spectral and diffuse flux reflected from Top surface
   gamma = (2 + x) / (1 + x)
   For i = 1 To Num
     Specular = TopSpec(i) * RspecTop(1, i) * (1 - Htop(1, i)) + TopDiff(i) * RspecTop(1, i) * (2 - gamma) * (1 - Htop(1, i))
     Diffuse = TopSpec(i) * RdiffTop(1, i) * Htop(1, i) + TopDiff(i) * RdiffTop(1, i) * ((gamma - 1) + Htop(1, i) * (2 - gamma))
     TopSpec(i) = Specular
     TopDiff(i) = Diffuse
   Next i
   
   'Update stored specular and diffuse brightness of Top and Bot surfaces
   For i = 1 To Num
       Delta = (SpecTop0(i) + TopSpec(i)) - SpecTopFront(i)
       If Math.Abs(Delta) > MaxDelta Then MaxDelta = Math.Abs(Delta)
       SpecTopFront(i) = SpecTopFront(i) + Delta
       SpecBotFront(i) = BotSpec(i)
       Delta = (DiffTop0(i) + TopDiff(i)) - DiffTopFront(i)
       If Math.Abs(Delta) > MaxDelta Then MaxDelta = Math.Abs(Delta)
       DiffTopFront(i) = DiffTopFront(i) + Delta
       DiffBotFront(i) = BotDiff(i)
   Next i
  
    
Else
'Calculate light trapping augmentation due to rear illumination
   
   'Calculate rear specular flux reaching Top surface at each element
   For i = 1 To Num
     If SpecBotRear(i) > MaxSource Then MaxSource = SpecBotRear(i)
     If alphaH < 10 Then  'Won't underflow exponential
       TopSpec(i) = SpecBotRear(i) * Exp(-alphaH)
     Else: TopSpec(i) = 0
     End If
   Next i
   
   For i = 1 To Num
     If DiffBotRear(i) > MaxSource Then MaxSource = DiffBotRear(i)
     Next i
     
   'Calculate rear diffuse flux reaching Top surface at each element
   DiffBotCol(1, 1) = DiffBotRear(1)     'Leftmost node
   For i = 2 To Num
     DiffBotCol(i, 1) = 0.5 * (DiffBotRear(i - 1) + DiffBotRear(i))
     Next i
   DiffBotCol(21, 1) = DiffBotRear(20)   'Rightmost node
   FDiffBotCol = Application.WorksheetFunction.MMult(RealtoFourier, DiffBotCol)
   For n = 0 To Num
     gamma = (2 + x) / (1 + x + 0.4 * Pi * n * A)
     If gamma * alphaH < 10 And n * A < 10 Then
       FDiffTopCol(n + 1, 1) = FDiffBotCol(n + 1, 1) * (2 / (1 + Exp(0.91 * Pi * n * A))) * Exp(-gamma * alphaH)
     Else: FDiffTopCol(n + 1, 1) = 0
     End If
     Next n
   DiffTopCol = Application.WorksheetFunction.MMult(FouriertoReal, FDiffTopCol)
   For i = 1 To Num
     TopDiff(i) = 0.5 * (DiffTopCol(i, 1) + DiffTopCol(i + 1, 1))
     Next i
   
   'Convert fluxes reaching Top surface to spectral and diffuse flux reflected from Top surface
   gamma = (2 + x) / (1 + x)
   For i = 1 To Num
     Specular = TopSpec(i) * RspecTop(1, i) * (1 - Htop(1, i)) + TopDiff(i) * RspecTop(1, i) * (2 - gamma) * (1 - Htop(1, i))
     Diffuse = TopSpec(i) * RdiffTop(1, i) * Htop(1, i) + TopDiff(i) * RdiffTop(1, i) * ((gamma - 1) + Htop(1, i) * (2 - gamma))
     TopSpec(i) = Specular
     TopDiff(i) = Diffuse
   Next i
   
   'Calculate reflected specular flux reaching Bot surface at each element
   For i = 1 To Num
     If alphaH < 10 Then  'Won't underflow exponential
       BotSpec(i) = TopSpec(i) * Exp(-alphaH)
     Else: BotSpec(i) = 0
     End If
   Next i
   
   'Calculate reflected diffuse flux reaching Bot surface at each element
   DiffTopCol(1, 1) = TopDiff(1)     'Leftmost node
   For i = 2 To Num
     DiffTopCol(i, 1) = 0.5 * (TopDiff(i - 1) + TopDiff(i))
     Next i
   DiffTopCol(21, 1) = TopDiff(20)   'Rightmost node
   FDiffTopCol = Application.WorksheetFunction.MMult(RealtoFourier, DiffTopCol)
   For n = 0 To Num
     gamma = (2 + x) / (1 + x + 0.4 * Pi * n * A)
     If gamma * alphaH < 10 And n * A < 10 Then
       FDiffBotCol(n + 1, 1) = FDiffTopCol(n + 1, 1) * (2 / (1 + Exp(0.91 * Pi * n * A))) * Exp(-gamma * alphaH)
     Else: FDiffBotCol(n + 1, 1) = 0
     End If
     Next n
   DiffBotCol = Application.WorksheetFunction.MMult(FouriertoReal, FDiffBotCol)
   For i = 1 To Num
     BotDiff(i) = 0.5 * (DiffBotCol(i, 1) + DiffBotCol(i + 1, 1))
     Next i
   
   'Convert fluxes reaching Bot surface to spectral and diffuse flux reflected from Bot surface
   gamma = (2 + x) / (1 + x)
   For i = 1 To Num
     Specular = BotSpec(i) * RspecBot(1, i) * (1 - Hbot(1, i)) + BotDiff(i) * RspecBot(1, i) * (2 - gamma) * (1 - Hbot(1, i))
     Diffuse = BotSpec(i) * RdiffBot(1, i) * Hbot(1, i) + BotDiff(i) * RdiffBot(1, i) * ((gamma - 1) + Hbot(1, i) * (2 - gamma))
     BotSpec(i) = Specular
     BotDiff(i) = Diffuse
   Next i
   
   'Update stored specular and diffuse brightness on Top and Bot surfaces
   For i = 1 To Num
       Delta = (SpecBot0(i) + BotSpec(i)) - SpecBotRear(i)
       If Math.Abs(Delta) > MaxDelta Then MaxDelta = Math.Abs(Delta)
       SpecBotRear(i) = SpecBotRear(i) + Delta
       SpecTopRear(i) = TopSpec(i)
       Delta = (DiffBot0(i) + BotDiff(i)) - DiffBotRear(i)
       If Math.Abs(Delta) > MaxDelta Then MaxDelta = Math.Abs(Delta)
       DiffBotRear(i) = DiffBotRear(i) + Delta
       DiffTopRear(i) = TopDiff(i)
   Next i
   
End If

If MaxDelta * MaxPasses < SmallDelta * MaxSource Then Exit For

Next iteration

For i = 1 To Num  'Load newly calculated surface brightness to local variables and to array for export to worksheet
  SpecTop0(i) = SpecTopFront(i) + SpecTopRear(i)
  SpecBot0(i) = SpecBotFront(i) + SpecBotRear(i)
  DiffTop0(i) = DiffTopFront(i) + DiffTopRear(i)
  DiffBot0(i) = DiffBotFront(i) + DiffBotRear(i)
  SpecTopTable(lambda, i + 4) = SpecTop0(i)
  SpecBotTable(lambda, i + 4) = SpecBot0(i)
  DiffTopTable(lambda, i + 4) = DiffTop0(i)
  DiffBotTable(lambda, i + 4) = DiffBot0(i)
Next i

'**** Calculate photogeneration at each node *****

'   ***Specular Light ***
  For j = 0 To Num
  'Downward flux
   Delta_y = j * H / Num
   SpecTopCol(1, 1) = SpecTop0(1)     'Leftmost node
   For i = 2 To Num
     SpecTopCol(i, 1) = 0.5 * (SpecTop0(i - 1) + SpecTop0(i))
     Next i
   SpecTopCol(Num + 1, 1) = SpecTop0(Num) 'Rightmost node
   FSpecTopCol = Application.WorksheetFunction.MMult(RealtoFourier, SpecTopCol)
   For n = 0 To Num
     If alpha * Delta_y < 10 Then
       FSpecBotCol(n + 1, 1) = alpha * FSpecTopCol(n + 1, 1) * Exp(-alpha * Delta_y)
       'Assuming exp(-alpha*y) dependence, adjust to exponential average around that node
       If j = 0 Then
         FSpecBotCol(n + 1, 1) = FSpecBotCol(n + 1, 1) * 2 * Num / (H * alpha) * (1 - Exp(-alpha * 0.5 * H / Num)) 'Top surface
       Else
         If j = Num Then
           FSpecBotCol(n + 1, 1) = FSpecBotCol(n + 1, 1) * 2 * Num / (H * alpha) * (Exp(alpha * 0.5 * H / Num) - 1) 'Bot surface
         Else
           FSpecBotCol(n + 1, 1) = FSpecBotCol(n + 1, 1) * Num / (H * alpha) * (Exp(alpha * 0.5 * H / Num) - Exp(-alpha * 0.5 * H / Num))
         End If
       End If
     Else: FSpecBotCol(n + 1, 1) = 0
     End If
     Next n
   SpecBotCol = Application.WorksheetFunction.MMult(FouriertoReal, FSpecBotCol)
   For i = 0 To Num
     PhotogenRate(j + 1, i + 1) = PhotogenRate(j + 1, i + 1) + SpecBotCol(i + 1, 1)
     Next i
  'Upward flux
   Delta_y = (Num - j) * H / Num
   SpecBotCol(1, 1) = SpecBot0(1)     'Leftmost node
   For i = 2 To Num
     SpecBotCol(i, 1) = 0.5 * (SpecBot0(i - 1) + SpecBot0(i))
     Next i
   SpecBotCol(Num + 1, 1) = SpecBot0(Num) 'Rightmost node
   FSpecBotCol = Application.WorksheetFunction.MMult(RealtoFourier, SpecBotCol)
   For n = 0 To Num
     If alpha * Delta_y < 10 Then
       FSpecTopCol(n + 1, 1) = alpha * FSpecBotCol(n + 1, 1) * Exp(-alpha * Delta_y)
       'Assuming exp(-alpha*(H-y)) dependence, adjust to exponential average around that node
       If j = Num Then
         FSpecTopCol(n + 1, 1) = FSpecTopCol(n + 1, 1) * 2 * Num / (H * alpha) * (1 - Exp(-alpha * 0.5 * H / Num)) 'Top surface
       Else
         If j = 0 Then
           FSpecTopCol(n + 1, 1) = FSpecTopCol(n + 1, 1) * 2 * Num / (H * alpha) * (Exp(alpha * 0.5 * H / Num) - 1) 'Bot surface
         Else
           FSpecTopCol(n + 1, 1) = FSpecTopCol(n + 1, 1) * Num / (H * alpha) * (Exp(alpha * 0.5 * H / Num) - Exp(-alpha * 0.5 * H / Num))
         End If
       End If
     Else: FSpecTopCol(n + 1, 1) = 0
     End If
     Next n
   SpecTopCol = Application.WorksheetFunction.MMult(FouriertoReal, FSpecTopCol)
   For i = 0 To Num
     PhotogenRate(j + 1, i + 1) = PhotogenRate(j + 1, i + 1) + SpecTopCol(i + 1, 1)
     Next i
  Next j
  
'    *** Diffuse light ***
  'Downward flux
  DiffTopCol(1, 1) = DiffTop0(1)     'Leftmost node
  For i = 2 To Num
    DiffTopCol(i, 1) = 0.5 * (DiffTop0(i - 1) + DiffTop0(i))
    Next i
  DiffTopCol(Num + 1, 1) = DiffTop0(Num) 'Rightmost node
  FDiffTopCol = Application.WorksheetFunction.MMult(RealtoFourier, DiffTopCol)
  
  For j = 0 To Num       'Calculate generation rate in each row for each harmonic, then transform to real space
    For n = 0 To Num
      Delta_y = j * H / Num
      x = 0.935 * Application.WorksheetFunction.Power(alpha * Delta_y, 0.67)
      gamma = (2 + x) / (1 + x + 0.47 * Pi * n * Delta_y / W)
      gamma_prime = gamma - 0.67 * x / (1 + x) ^ 2
      If gamma * alpha * Delta_y < 10 And n * Delta_y / W < 10 Then
        FDiffBotCol(n + 1, 1) = gamma_prime * alpha * FDiffTopCol(n + 1, 1) * (2 / (1 + Exp(0.88 * Pi * n * Delta_y / W))) * Exp(-gamma * alpha * Delta_y)
      Else: FDiffBotCol(n + 1, 1) = 0
      End If
      Next n
    DiffBotCol = Application.WorksheetFunction.MMult(FouriertoReal, FDiffBotCol)
    For i = 0 To Num
      DiffArray(j + 1, i + 1) = Application.WorksheetFunction.Max(0, DiffBotCol(i + 1, 1))
      Next i
    Next j
    
  Absorb_sum = 0   'Calculate total generation from downward flux across all nodes
  For j = 0 To Num
    For i = 0 To Num
      If (i = 0 Or i = Num) And (j = 0 Or j = Num) Then
        Absorb_sum = Absorb_sum + 0.25 * DiffArray(j + 1, i + 1) * H * W / Num ^ 2
      Else
        If (i = 0 Or i = Num Or j = 0 Or j = Num) Then
          Absorb_sum = Absorb_sum + 0.5 * DiffArray(j + 1, i + 1) * H * W / Num ^ 2
        Else
          Absorb_sum = Absorb_sum + DiffArray(j + 1, i + 1) * H * W / Num ^ 2
        End If
      End If
      Next i
    Next j
      
  'Calculate expected generation from downward flux (only need to consider n=0, since n>0 has net zero generation)
  x = 0.935 * Application.WorksheetFunction.Power(alphaH, 0.67)
  gamma = (2 + x) / (1 + x)
  If gamma * alphaH < 10 Then
    Absorb_calc = FDiffTopCol(1, 1) * (1 - Exp(-gamma * alphaH)) * W
  Else: Absorb_calc = FDiffTopCol(1, 1) * W
  End If
  
  'Fine-tune photogen to match expected total absorption
  For j = 0 To Num
    For i = 0 To Num
      If Absorb_sum > 0 Then PhotogenRate(j + 1, i + 1) = PhotogenRate(j + 1, i + 1) + DiffArray(j + 1, i + 1) * Absorb_calc / Absorb_sum
      Next i
    Next j
  
  'Upward flux
  DiffBotCol(1, 1) = DiffBot0(1)     'Leftmost node
  For i = 2 To Num
     DiffBotCol(i, 1) = 0.5 * (DiffBot0(i - 1) + DiffBot0(i))
     Next i
   DiffBotCol(Num + 1, 1) = DiffBot0(Num) 'Rightmost node
   FDiffBotCol = Application.WorksheetFunction.MMult(RealtoFourier, DiffBotCol)
   
  For j = 0 To Num       'Calculate generation rate in each row for each harmonic, then transform to real space
    For n = 0 To Num
      Delta_y = (Num - j) * H / Num
      x = 0.935 * Application.WorksheetFunction.Power(alpha * Delta_y, 0.67)
      gamma = (2 + x) / (1 + x + 0.47 * Pi * n * Delta_y / W)
      gamma_prime = gamma - 0.67 * x / (1 + x) ^ 2
      If gamma * alpha * Delta_y < 10 And n * Delta_y / W < 10 Then
        FDiffTopCol(n + 1, 1) = gamma_prime * alpha * FDiffBotCol(n + 1, 1) * (2 / (1 + Exp(0.88 * Pi * n * Delta_y / W))) * Exp(-gamma * alpha * Delta_y)
      Else: FDiffTopCol(n + 1, 1) = 0
      End If
      Next n
    DiffTopCol = Application.WorksheetFunction.MMult(FouriertoReal, FDiffTopCol)
    For i = 0 To Num
      DiffArray(j + 1, i + 1) = Application.WorksheetFunction.Max(0, DiffTopCol(i + 1, 1))
      Next i
    Next j
    
  Absorb_sum = 0   'Calculate total generation from upward flux across all nodes
  For j = 0 To Num
    For i = 0 To Num
      If (i = 0 Or i = Num) And (j = 0 Or j = Num) Then
        Absorb_sum = Absorb_sum + 0.25 * DiffArray(j + 1, i + 1) * H * W / Num ^ 2
      Else
        If (i = 0 Or i = Num Or j = 0 Or j = Num) Then
          Absorb_sum = Absorb_sum + 0.5 * DiffArray(j + 1, i + 1) * H * W / Num ^ 2
        Else
          Absorb_sum = Absorb_sum + DiffArray(j + 1, i + 1) * H * W / Num ^ 2
        End If
      End If
      Next i
    Next j
      
  'Calculate expected generation from upward flux (only need to consider n=0, since n>0 has net zero generation)
  x = 0.935 * Application.WorksheetFunction.Power(alphaH, 0.67)
  gamma = (2 + x) / (1 + x)
  If gamma * alphaH < 10 Then
    Absorb_calc = FDiffBotCol(1, 1) * (1 - Exp(-gamma * alphaH)) * W
  Else: Absorb_calc = FDiffBotCol(1, 1) * W
  End If
  
  'Fine-tune photogen to match expected total absorption
  For j = 0 To Num
    For i = 0 To Num
      If Absorb_sum > 0 Then PhotogenRate(j + 1, i + 1) = PhotogenRate(j + 1, i + 1) + DiffArray(j + 1, i + 1) * Absorb_calc / Absorb_sum
      Next i
    Next j

Next lambda

'Normalize photogeneration surrounding each node
PreFactor = (tau_N / q)
For i = 0 To Num
  For j = 0 To Num
    PhotogenRate(j + 1, i + 1) = PreFactor * PhotogenRate(j + 1, i + 1)
  Next j
Next i

'Copy results into worksheet. This is the brightness at each surface with light trapping included.
Range("Photogeneration!a297:x356") = SpecTopTable
Range("Photogeneration!a361:x420") = DiffTopTable
Range("Photogeneration!a425:x484") = SpecBotTable
Range("Photogeneration!a489:x548") = DiffBotTable
Range("Photogeneration!b15:v35") = PhotogenRate
End Sub
