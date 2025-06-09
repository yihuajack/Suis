import numpy as np
import pandas as pd
import matlab.engine
from scipy.optimize import minimize


VLimit = 1.2


def model(p, eng):
    deviceFile = r'E:\Documents\GitHub\ddmodel-octave\Input_files\niox_fapbi3_pcbm.csv'
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
        # command = ["matlab", "-batch", "run('E:/Documents/GitHub/ddmodel-octave/fast_test.m')"]
        # output = subprocess.run(command, capture_output=True, text=True, check=True)
        eng.run(r"E:/Documents/GitHub/ddmodel-octave/fast_test.m", nargout=0)

        JV_result = pd.read_csv(r'E:\Documents\GitHub\ddmodel-octave\JV.csv', header=None)
        return np.flip(JV_result[JV_result.iloc[:, 0] < VLimit].iloc[:, 1].to_numpy())   # 1 for RS 2 for FS
    except Exception as e:
        print("MATLAB Engine Error:", str(e))
        return None


def objective(p, x, y_interp, eng):
    y = model(p, eng)
    if y is None:
        print(f"Error in model evaluation with parameters: {p}")
        return 1e10
    loss = np.trapezoid(np.abs(y - y_interp), x)  # Integral Absolute Error (IAE)
    return loss


def main():
    eng = matlab.engine.start_matlab()

    x_all = pd.read_csv(r'E:\Documents\GitHub\ddmodel-octave\JV.csv', header=None).iloc[:, 0].to_numpy()
    x = np.flip(x_all[x_all < VLimit])

    JV_ref = pd.read_csv(r'../../data/optim/JV_NiOx_PVK_PCBM_1796092911782965264_BATTERY_253268_2024-05-27K1-5.csv')
    x_ref = JV_ref[JV_ref.iloc[:, 0] < VLimit].iloc[:, 0].to_numpy()
    y_ref = JV_ref[JV_ref.iloc[:, 0] < VLimit].iloc[:, 1].to_numpy()

    if x_ref[0] > x_ref[-1]:
        print("Warning: x_ref is not increasing, flipping it.")
        x_ref = np.flip(x_ref)
        y_ref = np.flip(y_ref)

    y_interp = np.interp(x, x_ref, y_ref)

    initial_guess = np.array([-6.0, -6.0, 2.0, 2.0, 2.0, 2.0, 17.0])
    bounds = [(-9.0, -5.0), (-9.0, -5.0), (0.0, 6.0), (0.0, 6.0), (0.0, 6.0), (0.0, 6.0), (15.0, 19.0)]

    result = minimize(
        lambda p: objective(p, x, y_interp, eng),
        initial_guess,
        bounds=bounds,
        method='L-BFGS-B',
        options={'maxiter': 20, 'disp': True, 'eps': 1e-2}
    )

    print(f"Optimal parameters: {result.x}")
    print(f"Minimum loss: {result.fun}")

    eng.quit()


if __name__ == "__main__":
    main()
