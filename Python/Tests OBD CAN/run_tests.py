import os
from dspicTest import dspicCommands

# === Test Mode ===
# Choose between: dspicCommands.TestScriptType.RecordResults or CompareResults
TestRun = dspicCommands.TestScriptType.RecordResults

# === JSON Output Path ===
JsonCompareFilePath = os.path.join(os.getcwd(), "ObdResults.json")

# === dsPIC COM Port Configuration ===
ComPortDspic = "COM8"   # ← Make sure dsPIC is on this port

# === Initialize Test Environment ===
print("🔌 Connecting to dsPIC33 on COM8...")
try:
    dspic = dspicCommands.DspicTestEnvironment(ComPortDspic, JsonCompareFilePath, TestRun)
except Exception as e:
    print(f"❌ Failed to connect: {e}")
    exit(1)

# === Run Command Tests ===
print("\n🚗 Starting OBD-II Command Tests...\n")

commands_to_test = [
    b'ATE0',     # Turn Echo Off
    b'010C',     # Engine RPM
    b'010D',     # Vehicle Speed
    b'0105',     # Coolant Temperature
    b'0111'      # Throttle Position
]

for cmd in commands_to_test:
    dspic.SimpleCommandForTest(cmd)

# === Save Results ===
print("\n💾 Saving results to ObdResults.json...\n")
dspic.Finish()

print("✅ Test finished successfully.")
