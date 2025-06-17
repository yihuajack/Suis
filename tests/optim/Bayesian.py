import numpy as np
import pandas as pd
import subprocess
from skopt import gp_minimize
from skopt.space import Real
from skopt.utils import use_named_args

# Define the search space
search_space = [
    Real(1e-6, 1e-5, name='dH'),
    Real(2e-5, 1e-4, name='dP'),
    Real(1e-6, 1e-5, name='dE')
]
#wd = r'E:\Documents\GitHub\ddmodel-octave'
wd = '~/simulation/device_module/ddmodel_octave'

def model(x):
    path = wd + r'/Input_files/niox_fapbi3_pcbm.csv'
    df = pd.read_csv(path)
    df.loc[1, 'thickness'] = x[0]
    df.loc[3, 'thickness'] = x[1]
    df.loc[5, 'thickness'] = x[2]
    df.to_csv(path, index=False)
    # Also see https://ww2.mathworks.cn/help/matlab/matlab-engine-for-python.html
    # command = ["matlab", "-batch", "run('E:/Documents/GitHub/ddmodel-octave/fast_test.m')"]
    command = ["flatpak", "-run", "org.octave.Octave", '--no-gui', '--quiet', '--eval',
               'cd {}'.format(wd),
               "demo_ms_pin({}, '{}/Libraries/Index_of_Refraction_library.xlsx')".format(path, wd)]

    try:
        output = subprocess.run(command, capture_output=True, text=True, check=True)
        print("MATLAB Output:\n", output.stdout)
        df_stats = pd.read_csv('{}/stats.csv'.format(wd), header=None)
        return df_stats.iloc[7, 1]  # 9 for eta

    except subprocess.CalledProcessError as e:
        print("Error occurred:", e.stderr)
        return None


# Objective function to minimize
@use_named_args(search_space)
def objective_function(**params):
    log_x = [params[key] for key in ['dH', 'dP', 'dE']]
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
