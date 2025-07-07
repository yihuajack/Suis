import numpy as np
import pandas as pd
import time
from scipy.optimize import minimize
try:
    import matlab.engine
    use_matlab_engine = True
    wd = r'E:\Documents\GitHub\ddmodel-octave'
except ModuleNotFoundError:
    import subprocess
    use_matlab_engine = False
    wd = '~/simulation/device_module/ddmodel_octave'


VLimit = 1.1
deviceFile = "{}/Input_files/niox_pk35_pcbm.csv".format(wd)
start_time = time.time()


def model_eng(p, eng):
    df = pd.read_csv(deviceFile)

    df.loc[3, 'taun'] = pow(10, p[0])
    df.loc[3, 'taup'] = pow(10, p[1])
    df.loc[2, 'sn'] = pow(10, p[2])
    df.loc[2, 'sp'] = pow(10, p[3])
    df.loc[4, 'sn'] = pow(10, p[4])
    df.loc[4, 'sp'] = pow(10, p[5])
    df.loc[3, 'Ncat'] = pow(10, p[6])
    df.loc[3, 'Nani'] = pow(10, p[6])

    df.to_csv(deviceFile, index=False)

    try:
        eng.run("{}/fast_test.m".format(wd), nargout=0)
        print(p)
        JV_result = pd.read_csv('{}/JV.csv'.format(wd), header=None)
        return np.flip(JV_result[JV_result.iloc[:, 0] < VLimit].iloc[:, 1].to_numpy())   # 1 for RS 2 for FS
    except Exception as e:
        print("MATLAB Engine Error:", str(e))
        return None


def model(p):
    df = pd.read_csv(deviceFile)

    df.loc[3, 'taun'] = pow(10, p[0])
    df.loc[3, 'taup'] = pow(10, p[1])
    df.loc[2, 'sn'] = pow(10, p[2])
    df.loc[2, 'sp'] = pow(10, p[3])
    df.loc[4, 'sn'] = pow(10, p[4])
    df.loc[4, 'sp'] = pow(10, p[5])
    df.loc[3, 'Ncat'] = pow(10, p[6])
    df.loc[3, 'Nani'] = pow(10, p[6])

    df.to_csv(deviceFile, index=False)
    command = [
        "flatpak", "run", "org.octave.Octave",
        # r"E:\Program Files\GNU Octave\Octave-9.2.0\mingw64\bin\octave-cli.exe",
        '--no-gui', '--quiet', '--eval',
        "cd('{}'), demo_ms_pin('{}', '{}/Libraries/Index_of_Refraction_library.xlsx')".format(wd, deviceFile, wd)]

    try:
        output = subprocess.run(command, capture_output=True, text=True, check=True)
        print("GNU Octave Output:\n", output.stdout)
        print(p)
        JV_result = pd.read_csv('{}/JV.csv'.format(wd), header=None)
        return np.flip(JV_result[JV_result.iloc[:, 0] < VLimit].iloc[:, 1].to_numpy())   # 1 for RS 2 for FS
    except subprocess.CalledProcessError as e:
        print("Error occurred:", e.stderr)
        print(command)
        return None


def objective_eng(p, x, y_interp, eng):
    y = model_eng(p, eng)
    if y is None:
        print(f"Error in model evaluation with parameters: {p}")
        return 1e10
    loss = np.trapezoid(np.abs(y - y_interp), x)  # Integral Absolute Error (IAE)
    return loss


def objective(p, x, y_interp):
    y = model(p)
    if y is None:
        print(f"Error in model evaluation with parameters: {p}")
        return 1e10
    loss = np.trapezoid(np.abs(y - y_interp), x)
    return loss


def callback_func(p):
    now = time.time()
    if callback_func.count == 0:
        callback_func.last_time = now
    elapsed = now - callback_func.last_time
    callback_func.count += 1
    total_elapsed = now - start_time
    print(f"[callback] Iteration {callback_func.count} | Time: {elapsed:.4f}s | Total: {total_elapsed:.4f}s | Params: {p}")
    callback_func.last_time = now

callback_func.count = 0
callback_func.last_time = None  # initialized in first call

def main():
    x_all = pd.read_csv('{}/JV.csv'.format(wd), header=None).iloc[:, 0].to_numpy()
    x = np.flip(x_all[x_all < VLimit])

    # Remember to check whether the file is read-only!
    JV_ref = pd.read_csv(r'../../data/optim/FTO-NiOx-PVKSAM-PCBM-BCP-Ag-253268_2024-05-31H1-2-1798229300897013761.csv')
    x_ref = JV_ref[JV_ref.iloc[:, 0] < VLimit].iloc[:, 0].to_numpy()
    y_ref = JV_ref[JV_ref.iloc[:, 0] < VLimit].iloc[:, 1].to_numpy()

    if x_ref[0] > x_ref[-1]:
        print("Info: x_ref is not increasing, flipping it.")
        x_ref = np.flip(x_ref)
        y_ref = np.flip(y_ref)

    y_interp = np.interp(x, x_ref, y_ref)

    initial_guess = np.array([-7.5, -7.5, 3.0, 3.0, 3.0, 3.0, 17.0])
    bounds = [(-9.0, -5.0), (-9.0, -5.0), (0.0, 6.0), (0.0, 6.0), (0.0, 6.0), (0.0, 6.0), (15.0, 19.0)]

    # Because the optimizer may call objective() multiple times per single "iteration" (e.g., probing different points),
    # this count reflects total calls, not strictly outer-loop iterations of the optimizer.
    # global counter
    if use_matlab_engine:
        eng = matlab.engine.start_matlab()
        result = minimize(
            lambda p: objective_eng(p, x, y_interp, eng),
            initial_guess,
            bounds=bounds,
            method='L-BFGS-B',
            options={'maxiter': 20, 'disp': True, 'eps': 1e-2},
            callback=callback_func
        )
        eng.quit()
    else:
        result = minimize(
            lambda p: objective(p, x, y_interp),
            initial_guess,
            bounds=bounds,
            method='L-BFGS-B',
            options={'maxiter': 20, 'disp': True, 'eps': 1e-2},
            callback=callback_func
        )

    print(f"Optimal parameters: {result.x}")
    print(f"Minimum loss: {result.fun}")


if __name__ == "__main__":
    main()
    total_time = time.time() - start_time
    print(f"[done] Optimization completed in {total_time:.4f} seconds with {callback_func.count} iterations.")
