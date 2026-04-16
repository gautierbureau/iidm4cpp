/**
 * Embedded mode example — Java orchestrator side.
 *
 * Accepts a single XIIDM file as argument. Loads the network, programmatically
 * adds CoordinatedReactiveControl to _GEN____3_SM if the generator exists but
 * the extension is absent, registers the network with IidmBridgeRegistry, calls
 * the C++ simulation via JNI, then reads back updated values before unregistering.
 *
 * Compile and run:
 *   javac -cp powsybl-core.jar:iidm-bridge-java.jar JavaLauncher.java
 *   java  -cp powsybl-core.jar:iidm-bridge-java.jar \
 *         -Djava.library.path=/path/to/libexample_embedded.so \
 *         JavaLauncher examples/IEEE14.xiidm
 */

import com.powsybl.iidm.network.*;
import com.powsybl.iidmbridge.jni.IidmBridgeRegistry;
import com.powsybl.iidm.network.extensions.ActivePowerControl;
import com.powsybl.iidm.network.extensions.CoordinatedReactiveControl;
import com.powsybl.iidm.network.extensions.CoordinatedReactiveControlAdder;
import com.powsybl.iidm.network.extensions.HvdcAngleDroopActivePowerControl;
import com.powsybl.iidm.network.extensions.HvdcOperatorActivePowerRange;
import com.powsybl.iidm.network.extensions.VoltagePerReactivePowerControl;
import com.powsybl.iidm.network.extensions.SlackTerminal;

import java.nio.file.Path;

public class JavaLauncher {

    // Loads libexample_embedded.so, which links iidmbridge and exports the JNI entry point.
    static {
        System.loadLibrary("example_embedded");
    }

    // JNI native method declaration — implemented in DynawoEmbedded.cpp
    private static native void runEmbedded(String networkId);

    public static void main(String[] args) throws Exception {
        if (args.length != 1) {
            System.err.println("Usage: JavaLauncher <path-to-network.xiidm>");
            System.exit(1);
        }

        processNetwork(args[0]);
    }

    private static void processNetwork(String filePath) throws Exception {
        // Load the network on the Java side
        Network network = Network.read(Path.of(filePath));
        System.out.println("[Java] Loaded network: " + network.getId());

        // Ensure _GEN____3_SM has CoordinatedReactiveControl so the JNI extension path is exercised
        Generator gen3 = network.getGenerator("_GEN____3_SM");
        if (gen3 != null && gen3.getExtension(CoordinatedReactiveControl.class) == null) {
            gen3.newExtension(CoordinatedReactiveControlAdder.class).withQPercent(40.0).add();
        }

        // Register using the network's own ID so C++ and Java agree on the key
        String networkId = network.getId();
        IidmBridgeRegistry.register(networkId, network);

        // Call the C++ simulation entry point
        System.out.println("[Java] Handing off to C++ simulation...");
        runEmbedded(networkId);
        System.out.println("[Java] C++ simulation returned.");

        // Read back generator values
        network.getGenerators().forEach(g ->
            System.out.printf("[Java] Generator %s targetP=%.1f MW%n",
                              g.getId(), g.getTargetP()));

        if (gen3 != null) {
            ActivePowerControl apc = gen3.getExtension(ActivePowerControl.class);
            if (apc != null)
                System.out.println("[Java] _GEN____3_SM activePowerControl.droop=" + apc.getDroop());
            CoordinatedReactiveControl crc = gen3.getExtension(CoordinatedReactiveControl.class);
            if (crc != null)
                System.out.println("[Java] _GEN____3_SM coordinatedReactiveControl.qPercent=" + crc.getQPercent());
        }

        // Read back HVDC extension values
        for (HvdcLine hvdc : network.getHvdcLines()) {
            HvdcAngleDroopActivePowerControl hadapc =
                hvdc.getExtension(HvdcAngleDroopActivePowerControl.class);
            if (hadapc != null)
                System.out.printf("[Java] HvdcLine %s hadapc: droop=%.4f p0=%.1f enabled=%s%n",
                                  hvdc.getId(), hadapc.getDroop(), hadapc.getP0(), hadapc.isEnabled());
            HvdcOperatorActivePowerRange hoar =
                hvdc.getExtension(HvdcOperatorActivePowerRange.class);
            if (hoar != null)
                System.out.printf("[Java] HvdcLine %s hoar: oprFromCS1toCS2=%.1f oprFromCS2toCS1=%.1f%n",
                                  hvdc.getId(), hoar.getOprFromCS1toCS2(), hoar.getOprFromCS2toCS1());
        }

        // Read back SVC extension values
        for (StaticVarCompensator svc : network.getStaticVarCompensators()) {
            VoltagePerReactivePowerControl vprc =
                svc.getExtension(VoltagePerReactivePowerControl.class);
            if (vprc != null)
                System.out.printf("[Java] StaticVarCompensator %s vprc.slope=%.4f%n",
                                  svc.getId(), vprc.getSlope());
        }

        // Read back SlackTerminal
        for (VoltageLevel vl : network.getVoltageLevels()) {
            SlackTerminal st = vl.getExtension(SlackTerminal.class);
            if (st != null) {
                Terminal term = st.getTerminal();
                Bus bus = term.getBusView().getConnectableBus();
                System.out.printf("[Java] VoltageLevel %s slackTerminal.busId=%s%n",
                                  vl.getId(), bus != null ? bus.getId() : "(unknown)");
            }
        }

        IidmBridgeRegistry.unregister(networkId);
    }
}
