import numpy as np
import matplotlib.pyplot as plt

# --- Default Configuration (can be overridden by main script if functions take args) ---
# These are used to define the shape of the Z data expected.
# The main script needs to be aware of these to generate Z correctly.
DEFAULT_N_R_BINS_DATA = 256//4
DEFAULT_N_THETA_BINS_DATA = 63

# Store meshgrid for data generation, calculated once during init
# Or, these could be returned by init_plot and passed to a separate data generation function.
# For simplicity here, let's assume the main script will generate data
# based on knowing these bin counts.
R_MESH_CENTERS_GLOBAL = None
THETA_MESH_CENTERS_GLOBAL = None


def get_data_generation_params(r_data_starts, r_data_ends,
                               theta_deg_min, theta_deg_max,
                               n_r_bins=DEFAULT_N_R_BINS_DATA,
                               n_theta_bins=DEFAULT_N_THETA_BINS_DATA):
    """
    Calculates and returns parameters needed for generating Z data of the correct shape.
    This includes the meshgrid of r and theta centers.
    """
    r_edges = np.linspace(r_data_starts, r_data_ends, n_r_bins + 1)
    theta_rad_min = np.deg2rad(theta_deg_min)
    theta_rad_max = np.deg2rad(theta_deg_max)
    theta_edges = np.linspace(theta_rad_min, theta_rad_max, n_theta_bins + 1)

    r_centers = (r_edges[:-1] + r_edges[1:]) / 2
    theta_centers = (theta_edges[:-1] + theta_edges[1:]) / 2
    R_mesh, Theta_mesh = np.meshgrid(r_centers, theta_centers, indexing='ij')

    return {
        "r_edges": r_edges,
        "theta_edges": theta_edges,
        "R_mesh_centers": R_mesh,
        "Theta_mesh_centers": Theta_mesh,
        "n_r_bins": n_r_bins,
        "n_theta_bins": n_theta_bins
    }


def init_polar_heatmap(initial_Z_data,
                       r_data_starts=1.0, r_data_ends=5.0,
                       theta_deg_min=-35, theta_deg_max=35,
                       r_axis_display_min=0.0, r_axis_display_max=5.0,
                       n_r_bins=DEFAULT_N_R_BINS_DATA,
                       n_theta_bins=DEFAULT_N_THETA_BINS_DATA,
                       cmap='viridis'):
    """
    Initializes and displays a polar heatmap.

    Args:
        initial_Z_data (np.array): The initial 2D data array for the heatmap.
                                   Shape must be (n_r_bins, n_theta_bins).
        r_data_starts (float): Radial start for the colored data.
        r_data_ends (float): Radial end for the colored data.
        theta_deg_min (float): Min angle for data (degrees).
        theta_deg_max (float): Max angle for data (degrees).
        r_axis_display_min (float): Min radius for the plot axis.
        r_axis_display_max (float): Max radius for the plot axis.
        n_r_bins (int): Number of radial bins for Z data.
        n_theta_bins (int): Number of angular bins for Z data.
        cmap (str): Colormap to use.

    Returns:
        dict: A dictionary containing handles to 'fig', 'ax', 'pcm', and 'plot_title'.
    """
    if initial_Z_data.shape != (n_r_bins, n_theta_bins):
        raise ValueError(f"initial_Z_data shape {initial_Z_data.shape} "
                         f"does not match n_r_bins ({n_r_bins}), n_theta_bins ({n_theta_bins})")

    plt.ion()  # Turn on interactive mode

    fig, ax = plt.subplots(figsize=(16, 9), subplot_kw={'projection': 'polar'})

    # Define bin EDGES for pcolormesh based on data parameters
    r_edges = np.linspace(r_data_starts, r_data_ends, n_r_bins + 0)
    theta_rad_min_val = np.deg2rad(theta_deg_min)
    theta_rad_max_val = np.deg2rad(theta_deg_max)
    theta_edges = np.linspace(theta_rad_min_val, theta_rad_max_val, n_theta_bins + 0)

    pcm = ax.pcolormesh(theta_edges, r_edges, initial_Z_data, shading='gouraud', cmap=cmap)

    # Customize Plot
    # print("theta_min ", theta_deg_min, " theta_max ", theta_deg_max)
    # print("r_min ", r_axis_display_min, " r_max ", r_axis_display_max)
    ax.set_thetamin(theta_deg_min)
    ax.set_thetamax(theta_deg_max)
    ax.set_rmin(r_axis_display_min)
    ax.set_rmax(r_axis_display_max)
    ax.set_theta_zero_location("N")
    ax.set_theta_direction(-1)
    ax.tick_params(labelsize=14)

    
    ax.annotate('R(m)', xy=(theta_rad_min_val, ax.get_ylim()[1]/2), xytext=(np.pi/-2.9, ax.get_ylim()[1]/2), ha='center', va='center', fontsize=14, color='black')  
    
    # Range axis label in mid distance outside of plot, use xytext to have text outside of plotted area
    ax.set_xlabel("Azimuth angle (°)", fontsize=14)
    ax.xaxis.set_label_coords(0.5, 0.95)
    
    plot_title_obj = ax.set_title("Sensing Heatmap", fontsize=18)
    #plot_title_obj = ax.set_title(
    #    f'Frame 0\nData: r={r_data_starts}-{r_data_ends}, Angle: {theta_deg_min}° to {theta_deg_max}°',
    #    va='bottom', pad=20
    #)
    #ax.set_ylabel('Radius', labelpad=30)

    ax.grid(True, linestyle='--', alpha=0.7)
    radial_grid_values = np.arange(r_data_starts, r_axis_display_max + 0.1, 1.0) # +0.1 to include r_axis_display_max
    if r_data_starts > r_axis_display_min : # Only set rgrids if data doesn't start at axis min
        ax.set_rgrids(radial_grid_values)
    else: # If data starts at axis min, let rgrids be default or set from r_axis_display_min
        ax.set_rgrids(np.arange(r_axis_display_min, r_axis_display_max + 0.1, 1.0))


    fig.canvas.draw_idle()
    # plt.show(block=False)

    return {
        "fig": fig,
        "ax": ax,
        "pcm": pcm,
        "plot_title": plot_title_obj,
        # Pass back config for consistent title updates if needed
        "r_data_starts": r_data_starts, "r_data_ends": r_data_ends, "n_r_bins": n_r_bins,
        "theta_deg_min": theta_deg_min, "theta_deg_max": theta_deg_max, "n_theta_bins": n_theta_bins
    }


def update_polar_heatmap(plot_handles, new_Z_data, frame_count):
    """
    Updates an existing polar heatmap with new data.

    Args:
        plot_handles (dict): Dictionary returned by init_polar_heatmap.
        new_Z_data (np.array): The new 2D data array. Shape must match initial.
        frame_count (int): Current frame number for the title.
    """
    pcm = plot_handles["pcm"]
    fig = plot_handles["fig"]
    plot_title_obj = plot_handles["plot_title"]

    # Check if plot window still exists
    if not plt.fignum_exists(fig.number):
        # print("Plot window closed. Cannot update.") # Optional: print a message
        return False # Indicate update failed

    expected_shape = pcm.get_array().reshape(plot_handles["n_r_bins"], plot_handles["n_theta_bins"]).shape # Infer shape
    if new_Z_data.shape != expected_shape:
         raise ValueError(f"new_Z_data shape {new_Z_data.shape} "
                         f"does not match expected shape {expected_shape}")


    pcm.set_array(new_Z_data.ravel())

    # Optional: Dynamically adjust color limits
    # current_min = new_Z_data.min()
    # current_max = new_Z_data.max()
    # if current_min != current_max:
    #     pcm.set_clim(vmin=current_min, vmax=current_max)
    # else:
    #     pcm.set_clim(vmin=current_min - 0.1, vmax=current_max + 0.1) # Avoid flat clim

    # Update title using stored config from plot_handles
    cfg = plot_handles
    #plot_title_obj.set_text(
    #    f'Frame {frame_count}\nData: r={cfg["r_data_starts"]}-{cfg["r_data_ends"]}, Angle: {cfg["theta_deg_min"]}° to {cfg["theta_deg_max"]}°'
    #)

    fig.canvas.draw_idle()
    # plt.pause(0.001) # Crucial for GUI to update in script mode
    return True # Indicate update success


def close_plot(plot_handles):
    """
    Turns off interactive mode and closes the plot.
    """
    if plot_handles and plt.fignum_exists(plot_handles["fig"].number):
        plt.close(plot_handles["fig"])
    plt.ioff()


if __name__ == '__main__':
    # --- Example of using the plotter module itself ---
    print("Testing polar_plotter.py directly...")

    # Config for test
    test_r_data_starts = 1.0
    test_r_data_ends = 5.0
    test_theta_deg_min = -45
    test_theta_deg_max = 45
    test_n_r_bins = 20
    test_n_theta_bins = 50

    # Get parameters for data generation
    data_params = get_data_generation_params(
        test_r_data_starts, test_r_data_ends,
        test_theta_deg_min, test_theta_deg_max,
        n_r_bins=test_n_r_bins, n_theta_bins=test_n_theta_bins
    )
    R_mesh = data_params["R_mesh_centers"]
    Theta_mesh = data_params["Theta_mesh_centers"]

    def generate_sample_data(time_step, R, Theta):
        phase = time_step * 0.1
        return np.sin(3 * Theta + phase) * np.exp(-(R - (test_r_data_starts + test_r_data_ends)/2 - np.sin(phase*2))**2 / 2)

    initial_Z = generate_sample_data(0, R_mesh, Theta_mesh)

    handles = init_polar_heatmap(initial_Z,
                                 r_data_starts=test_r_data_starts, r_data_ends=test_r_data_ends,
                                 theta_deg_min=test_theta_deg_min, theta_deg_max=test_theta_deg_max,
                                 n_r_bins=test_n_r_bins, n_theta_bins=test_n_theta_bins)

    try:
        for i in range(1, 30):
            if not plt.fignum_exists(handles["fig"].number):
                break
            new_Z = generate_sample_data(i, R_mesh, Theta_mesh)
            update_polar_heatmap(handles, new_Z, i)
            import time
            time.sleep(0.1)
    except KeyboardInterrupt:
        print("Test interrupted.")
    finally:
        if handles and plt.fignum_exists(handles["fig"].number):
            print("Test finished. Close plot window to exit.")
            plt.show() # Keep open
        close_plot(handles) # Ensure ioff is called
