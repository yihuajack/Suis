import numpy as np
import pandas as pd
import subprocess
from scipy.optimize import minimize


def model(p, start_row):
    df = pd.read_csv(r'E:\Documents\GitHub\ddmodel-octave\Input_files\spiro_mapi_tio2_custom.csv')
    df.loc[3, 'taun'] = pow(10, p[0])
    df.loc[3, 'taup'] = pow(10, p[1])
    df.loc[2, 'sn'] = pow(10, p[2])
    df.loc[4, 'sp'] = pow(10, p[3])
    df.to_csv(r'E:\Documents\GitHub\ddmodel-octave\Input_files\spiro_mapi_tio2_custom.csv', index=False)
    # Also see https://ww2.mathworks.cn/help/matlab/matlab-engine-for-python.html
    command = ["matlab", "-batch", "run('E:/Documents/GitHub/ddmodel-octave/demo_ms_pin.m')"]

    try:
        output = subprocess.run(command, capture_output=True, text=True, check=True)
        print("MATLAB Output:\n", output.stdout)
        return pd.read_csv(r'E:\Documents\GitHub\ddmodel-octave\JV.csv', header=None).iloc[start_row:, 1].to_numpy()

    except subprocess.CalledProcessError as e:
        print("Error occurred:", e.stderr)


# Reference curve
y_ref = pd.read_csv('../../data/optim/spiro_mapi_tio2_custom_jv_ref.csv', header=None).iloc[:, 1].to_numpy()


# Objective function: Minimize the difference between output and reference curve
def objective(p):
    y = model(p, start_row=19)  # starting from the 20th row
    return np.mean((y - y_ref) ** 2)  # Mean Squared Error


# Initial guess for the parameters (can be fine-tuned)
initial_guess = np.array([-9.0, -7.0, 1.0, 3.0])
bounds = [(-10.0, -1.0), (-10.0, -1.0), (0.0, 9.0), (0.0, 9.0)]

# Perform optimization
# BFGS: unconstrained, does not have any bounds or constraints
# L-BFGS-B: (variable) bounds (bound constraints)
# SLSQP: (general equality or inequality) constraints, handles both bounds and general constraints
result = minimize(objective, initial_guess, bounds=bounds, method='L-BFGS-B', options={'maxiter': 50, 'disp': True, 'eps': 1e-3})

# Print the optimal parameters and the loss value
print(f"Optimal parameters: {result.x}")
print(f"Minimum loss: {result.fun}")
