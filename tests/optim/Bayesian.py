import numpy as np
import pandas as pd
import subprocess
from skopt import gp_minimize
from skopt.space import Real
from skopt.utils import use_named_args

# Define the search space
search_space = [
    Real(1e-5, 5e-5, name='dE'),
    Real(1e-5, 5e-5, name='dP'),
    Real(5e-6, 2e-5, name='dH')
]

def model(x):
    df = pd.read_csv(r'E:\Documents\GitHub\ddmodel-octave\Input_files\spiro_mapi_tio2_custom.csv')
    df.loc[1, 'thickness'] = x[0]
    df.loc[3, 'thickness'] = x[1]
    df.loc[5, 'thickness'] = x[2]
    df.to_csv(r'E:\Documents\GitHub\ddmodel-octave\Input_files\spiro_mapi_tio2_custom.csv', index=False)
    # Also see https://ww2.mathworks.cn/help/matlab/matlab-engine-for-python.html
    command = ["matlab", "-batch", "run('E:/Documents/GitHub/ddmodel-octave/fast_test.m')"]

    try:
        output = subprocess.run(command, capture_output=True, text=True, check=True)
        print("MATLAB Output:\n", output.stdout)
        df_stats = pd.read_csv(r'E:\Documents\GitHub\ddmodel-octave\stats.csv', header=None)
        return df_stats.iloc[9, 1]

    except subprocess.CalledProcessError as e:
        print("Error occurred:", e.stderr)

# Objective function to minimize
@use_named_args(search_space)
def objective_function(**params):
    log_x = [params[key] for key in ['dE', 'dP', 'dH']]
    y = model(log_x)  # Get outputs from the black-box model
    loss = -y ** 2
    return loss

# Run Bayesian Optimization with Gaussian Process
result = gp_minimize(
    func=objective_function,        # Objective function
    dimensions=search_space,        # Search space
    n_calls=20,                     # Number of evaluations
    random_state=42,                # For reproducibility
    acq_func='EI'                   # Acquisition function: Expected Improvement
)

# Extract the best inputs found
best_log_inputs = result.x
best_inputs = np.array(best_log_inputs)

print("Best inputs:", best_inputs)
print("Best objective value:", result.fun)
