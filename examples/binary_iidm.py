import pypowsybl as pp
import sys

def create_binary_iidm(xiidm_path):
    network = pp.network.load(xiidm_path)
    if xiidm_path.endswith('.xiidm'):
        output_path = xiidm_path.replace('.xiidm', '.biidm')
        network.save(output_path, format='BIIDM')

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python binary_iidm.py <xiidm_file>")
        sys.exit(1)
    xiidm_path = sys.argv[1]
    create_binary_iidm(xiidm_path)