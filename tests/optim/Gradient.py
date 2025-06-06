import numpy as np
import pandas as pd
import subprocess
from scipy.optimize import minimize


def model(p):
    deviceFile = r'E:\Documents\GitHub\ddmodel-octave\Input_files\niox_fapbi3_pcbm.csv'
    df = pd.read_csv(deviceFile)
    df.loc[3, 'taun'] = pow(10, p[0])
    df.loc[3, 'taup'] = pow(10, p[1])
    df.loc[2, 'sn'] = pow(10, p[2])
    df.loc[4, 'sp'] = pow(10, p[3])
    df.loc[3, 'Ncat'] = pow(10, p[4])
    df.loc[3, 'Nani'] = pow(10, p[4])
    df.to_csv(deviceFile, index=False)
    # Also see https://ww2.mathworks.cn/help/matlab/matlab-engine-for-python.html
    command = ["matlab", "-batch", "run('E:/Documents/GitHub/ddmodel-octave/fast_test.m')"]

    try:
        output = subprocess.run(command, capture_output=True, text=True, check=True)
        print("MATLAB Output:\n", output.stdout)
        print(p)
        JV_result = pd.read_csv(r'E:\Documents\GitHub\ddmodel-octave\JV.csv', header=None)
        return np.flip(JV_result[JV_result.iloc[:, 0] < VLimit].iloc[:, 1].to_numpy())   # 1 for RS 2 for FS

    except subprocess.CalledProcessError as e:
        print("Error occurred:", e.stderr)
        return None


# Reference curve
VLimit = 1.1
x = pd.read_csv(r'E:\Documents\GitHub\ddmodel-octave\JV.csv', header=None).iloc[:, 0].to_numpy()
x = np.flip(x[x < VLimit])
JV_ref = pd.read_csv('../../data/optim/IV_NiOx_PVK_PCBM_1796092903998337025_BATTERY_253268_2024-05-27A1-1.csv')
x_ref = JV_ref[JV_ref.iloc[:, 0] < VLimit].iloc[:, 0].to_numpy()  # xp must be increasing
y_ref = JV_ref[JV_ref.iloc[:, 0] < VLimit].iloc[:, 1].to_numpy()
if x_ref[0] > x_ref[-1]:
    print("Warning: x_ref is not increasing, flipping it.")
    x_ref = np.flip(x_ref)
    y_ref = np.flip(y_ref)

y_interp = np.interp(x, x_ref, y_ref)  # interpolated reference curve


# Objective function: Minimize the difference between output and reference curve
def objective(p):
    y = model(p)  # starting from the 24th (RS 1V) / 41st (FS) row
    if y is None:
        print(f"Error in model evaluation with parameters: {p}")
        return 1e10

    # loss = np.mean((y - y_interp) ** 2)  # Mean Squared Error (MSE)
    loss = np.trapezoid(np.abs(y - y_interp), x)  # Integral Absolute Error (IAE)
    return loss


# Initial guess for the parameters (can be fine-tuned)
initial_guess = np.array([-7.0, -7.0, 3.0, 3.0, 18.0])
bounds = [(-9.0, -1.0), (-9.0, -1.0), (0.0, 9.0), (0.0, 9.0), (15.0, 19.0)]

# Perform optimization
# BFGS: unconstrained, does not have any bounds or constraints
# L-BFGS-B: (variable) bounds (bound constraints)
# SLSQP: (general equality or inequality) constraints, handles both bounds and general constraints
result = minimize(objective, initial_guess, bounds=bounds, method='L-BFGS-B', options={'maxiter': 20, 'disp': True, 'eps': 1e-2})

# Print the optimal parameters and the loss value
print(f"Optimal parameters: {result.x}")
print(f"Minimum loss: {result.fun}")
