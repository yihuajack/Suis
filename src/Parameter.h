enum class LayerType {LAYER, ACTIVE, JUNCTION};
enum class VFunType {CONSTANT, SWEEPANDSTILL, SIN, SWEEP};
enum class G1FunType {CONSTANT, SWEEPANDSTILL, SIN, SWEEP};
enum class G2FunType {CONSTANT, SWEEPANDSTILL, SQAURE};
enum class TMeshType {LINEAR, LOG10, LOG10_DOUBLE};

class Parameter {
public:
    // Physical constants
    const double kB = 8.617330350e-5;  // Boltzmann constant [eV K^-1]
    const double eps0 = 552434;  // Epsilon_0 [e^2 eV^-1 cm^-1] - Checked (02-11-15)
    const double q = 1;  // Charge of the species in units of e.
    const double e = 1.60217662e-19;  // Elementary charge in Coulombs.

    double T = 300;  // Temperature [K]

    // Spatial mesh
    double d = 400e-7;  // Layer and subsection thickness array
    std::size_t layer_points = 400;  // Points array

    // Layer description
    /*
     * Define the layer type for each of the layers in the device.
     * The options are:
     * LAYER = standard layer
     * ACTIVE = standard layer but the properties of this layer are
     * flagged such that they can easily be accessed
     * JUNCTION = a region with graded properties between two materials
     * (either LAYER or ACTIVE type)
     * with different properties
     */
    LayerType layer_type = LayerType::ACTIVE;
    std::string material = "MAPbICl";

    /* Time mesh */
    // 1 for linear, 2 for log10
    TMeshType tmesh_type = TMeshType::LOG10;  // Mesh type - for use with meshgen_t
    double t0 = 1e-16;  // Initial log mesh time value
    double tmax = 1e-12;  // Max time value
    std::size_t tpoints = 100;  // Number of time points

    // GENERAL CONTROL PARAMETERS
    bool mobset = true;  // Switch on/off electron hole mobility - MUST BE SET TO ZERO FOR INITIAL SOLUTION
    bool mobseti = true;  // Switch on/off ionic carrier mobility - MUST BE SET TO ZERO FOR INITIAL SOLUTION
    bool SRHset = true;  // Switch on/off SRH recombination - recommend setting to zero for initial solution
    bool radset = true;  // Switch on/off band-to-band recombination

    /* Generation */
    // false = Uniform Generation
    // true = Beer Lambert
    bool optical_model = true;
    double int1 = 0;  // Light intensity source 1 (multiples of g0 or 1 sun for Beer-Lambert)
    double int2 = 0;  // Light intensity source 2 (multiples of g0 or 1 sun for Beer-Lambert)
    double g0 = 2.6409e+21;  // Uniform generation rate [cm-3s-1]
    std::string light_source1 = "AM15";
    std::string light_source2 = "laser";
    double laser_lambda1 = 0;
    double laser_lambda2 = 638;
    G1FunType g1_fun_type = G1FunType::CONSTANT;
    G2FunType g2_fun_type = G2FunType::CONSTANT;
    double g1_fun_arg = 0;
    double g2_fun_arg = 0;
    bool side = false;  // illumination side false = left, true = right
    // default: Approximate Uniform generation rate @ 1 Sun for 510 nm active layer thickness

    // Mobile ions
    std::size_t N_ionic_species = 1;

    // Volumetric surface recombination
    bool vsr_mode = true;  // Either true for volumetric surface recombination approximation or false for off
    bool vsr_check = true;  // Perform check for self-consistency at the end of DF

    /* Series resistance */
    double Rs = 0;
    double Rs_initial = 0;  // Switch to allow linear ramp of Rs on first application

    /* Voltage function parameters */
    VFunType V_fun_type = VFunType::CONSTANT;
    double V_fun_arg = 0;
};
