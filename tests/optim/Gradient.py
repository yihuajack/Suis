import numpy as np
import pandas as pd
import subprocess
from scipy.optimize import minimize


def model(p, start_row):
    df = pd.read_csv(r'E:\Documents\GitHub\ddmodel-octave\Input_files\spiro_fapbi3_tio2_opt.csv')
    df.loc[3, 'taun'] = pow(10, p[0])
    df.loc[3, 'taup'] = pow(10, p[1])
    df.loc[2, 'sn'] = pow(10, p[2])
    df.loc[4, 'sp'] = pow(10, p[3])
    df.loc[2, 'Ncat'] = pow(10, p[4])
    df.loc[2, 'Nani'] = pow(10, p[4])
    df.loc[3, 'Ncat'] = pow(10, p[4])
    df.loc[3, 'Nani'] = pow(10, p[4])
    df.loc[4, 'Ncat'] = pow(10, p[4])
    df.loc[4, 'Nani'] = pow(10, p[4])
    df.to_csv(r'E:\Documents\GitHub\ddmodel-octave\Input_files\spiro_fapbi3_tio2_opt.csv', index=False)
    # Also see https://ww2.mathworks.cn/help/matlab/matlab-engine-for-python.html
    command = ["matlab", "-batch", "run('E:/Documents/GitHub/ddmodel-octave/demo_ms_pin.m')"]

    try:
        output = subprocess.run(command, capture_output=True, text=True, check=True)
        print("MATLAB Output:\n", output.stdout)
        return pd.read_csv(r'E:\Documents\GitHub\ddmodel-octave\JV.csv', header=None).iloc[start_row:, 1].to_numpy()  # 1 for RS 2 for FS

    except subprocess.CalledProcessError as e:
        print("Error occurred:", e.stderr)


# Reference curve
x = pd.read_csv(r'E:\Documents\GitHub\ddmodel-octave\JV.csv', header=None).iloc[23:, 0].to_numpy()
x_ref = pd.read_csv('../../data/optim/spiro_fapbi3_tio2_rs_ref.csv').iloc[:, 0].to_numpy()
y_ref = pd.read_csv('../../data/optim/spiro_fapbi3_tio2_rs_ref.csv').iloc[:, 1].to_numpy()
y_interp = np.interp(x, x_ref, y_ref)


lambda_ = 0.1
# Objective function: Minimize the difference between output and reference curve
def objective(p):
    y = model(p, start_row=23)  # starting from the 24th (RS 1V) / 41st (FS) row
    mse_loss = np.mean((y - y_interp) ** 2)  # Mean Squared Error
    dy = np.gradient(y, x)
    dy_interp = np.gradient(y_interp, x)
    mse_loss_grad = np.mean((dy - dy_interp) ** 2)  # Gradient loss
    return mse_loss# + lambda_ * mse_loss_grad


# Initial guess for the parameters (can be fine-tuned)
initial_guess = np.array([-7.0, -7.0, 3.0, 3.0, 18.0])
bounds = [(-10.0, -1.0), (-10.0, -1.0), (0.0, 9.0), (0.0, 9.0), (15.0, 19.0)]

# Perform optimization
# BFGS: unconstrained, does not have any bounds or constraints
# L-BFGS-B: (variable) bounds (bound constraints)
# SLSQP: (general equality or inequality) constraints, handles both bounds and general constraints
result = minimize(objective, initial_guess, bounds=bounds, method='L-BFGS-B', options={'maxiter': 20, 'disp': True, 'eps': 1e-6})

# Print the optimal parameters and the loss value
print(f"Optimal parameters: {result.x}")
print(f"Minimum loss: {result.fun}")
