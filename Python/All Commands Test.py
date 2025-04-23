import serial
import time
from datetime import datetime
import os

# === Configuration ===
COM_PORT = 'COM9'  # Replace with your actual COM port
BAUD_RATE = 9600
COMMAND_DELAY = 0.5
RESPONSE_IDLE_TIMEOUT = 1.0
POST_TEST_DELAY = 1.5

# === Log File Setup ===
desktop_path = os.path.join("C:\\", "Users", "irtaz", "OneDrive", "Desktop")
log_filename = os.path.join(desktop_path, f"command_test_log_{datetime.now().strftime('%Y%m%d_%H%M%S')}.txt")

# === Commands to Test ===
commands_to_test = [
    "ATI", "ATD", "ATWS", "ATZ", "ATCS", "ATNL", "ATPOC", "ATPPS", "ATPPR", "AT@1",
    "ATV 1", "ATV 0", "ATFCSM 1", "ATFCSM 0", "ATD 1", "ATD 0", "ATS 1", "ATS 0", "ATH 1", 
    "ATH 0", "ATAT 1", "ATAT 0", "ATCFC 1", "ATCFC 0", "ATCAF 1", "ATCAF 0", "ATR 1", "ATR 0", 
    "ATL 1", "ATL 0", "ATE 1", "ATE 0", 
    "ATPB E0 04", "ATFCSH 7E0", "ATFCSH 18DAF110", "ATFCSD 30 02 10", 
    "ATCRA 7E9", "ATCRA 18DAF110", "ATCRA 7EX", "ATCM 7F0", "ATCM FFFFFF00", 
    "ATCF 7E0", "ATCF 18DAF110", "STGPOW 1:1, 2:0", "STGPOR 1, 2", "STGPIR 1, 2", "STGPIRH 1, 2",
    "STFPL", "STGPC", "STUIC", "STSAVCAL", "STRSTNVM", "STCALSTAT", "STFPA", "STFFCA", "STFBC",
    "STFBA", "STFA", "STFAC", "STMA", "STM", "STCTR", "STCTRR", "STCTOR", "STCSTM", "STCSEGT",
    "STCSEGR", "STCMM", "STCFCPC", "STCFCPA", "STP", "STPX", "STPTO", "STPTRQ", "STPTOT", "STPR",
    "STPRS", "STPC", "STPCB", "STPBR", "STPBRR", "STVR", "STVRX", "STVCAL", "STS@1", "STSN",
    "STSDI", "STSATI", "STI", "STDICPO", "STDI", "STWBR", "STUFC", "STSLLT"  # Added STSLLT here
]

# === Helper Functions ===
def generate_variations(cmd):
    """ Generate variations of commands to handle multiple cases (if applicable) """
    if cmd.startswith("AT") and len(cmd.split()) == 1:
        return [cmd]  # No parameters, just the command itself
    if cmd.startswith("AT") and len(cmd.split()) == 2:
        return [cmd, f"{cmd}9", f"{cmd}    "]  # Add invalid and multiple spaces for testing
    return [cmd]  # For more complex commands

def send_command(ser, cmd):
    """ Send command via serial and return the response """
    try:
        ser.reset_input_buffer()
        ser.write(cmd.encode('utf-8') + b'\r')
        time.sleep(COMMAND_DELAY)
        response = b""
        last_read_time = time.time()
        while True:
            if ser.in_waiting > 0:  
                response += ser.read(ser.in_waiting)
                last_read_time = time.time()
            elif time.time() - last_read_time > RESPONSE_IDLE_TIMEOUT:
                break
            time.sleep(0.05)
        decoded = response.decode(errors='ignore')
        return decoded if decoded.strip() else "[NO RESPONSE RECEIVED]"
    except Exception as e:
        return f"[ERROR: {str(e)}]"

def evaluate_response(cmd_input, response, test_index):
    """ Evaluate the response based on the command """
    clean = response.strip()
    if test_index == 0:  # valid case
        return "PASS" if clean and "?" not in clean else "FAIL"
    if test_index == 1:  # invalid case, expects ?
        return "PASS" if "?" in clean else "FAIL"
    if test_index == 2:  # multiple spaces
        return "PASS" if clean and "?" not in clean else "FAIL"
    return "FAIL"

def log_result(f, label, cmd_input, response, result):
    """ Log the results to a file """
    f.write(f"--- {label} --- [{result}]\n")
    f.write(f"Command Sent: {repr(cmd_input)}\n")
    f.write(f"Response:\n{response}\n\n")
    f.flush()

# === Main Test Routine ===
def run_test():
    """ Main function to run the test and log results """
    working_count = 0
    not_working_count = 0

    with serial.Serial(COM_PORT, BAUD_RATE, timeout=1) as ser, open(log_filename, 'w') as log_file:
        log_file.write(f"=== Command Test Log: {datetime.now()} ===\n\n")

        # Test all commands
        for command in commands_to_test:
            log_file.write(f"\n=== Testing Command: {command} ===\n")
            log_file.flush()

            # Case 1: Valid command
            result_valid = "FAIL"
            response_valid = send_command(ser, command)
            result_valid = evaluate_response(command, response_valid, 0)
            log_result(log_file, f"{command} Test Valid", command, response_valid, result_valid)
            print(f"{command} Test Valid: {result_valid} — {response_valid.strip()[:80] or '[EMPTY RESPONSE]'}")

            # Case 2: Invalid command (invalid param or variation)
            invalid_command = command + "9"
            result_invalid = "FAIL"
            response_invalid = send_command(ser, invalid_command)
            result_invalid = evaluate_response(invalid_command, response_invalid, 1)
            log_result(log_file, f"{command} Test Invalid", invalid_command, response_invalid, result_invalid)
            print(f"{command} Test Invalid: {result_invalid} — {response_invalid.strip()[:80] or '[EMPTY RESPONSE]'}")

            # Case 3: Multiple spaces in command
            command_with_spaces = command + "    "
            result_spaces = "FAIL"
            response_spaces = send_command(ser, command_with_spaces)
            result_spaces = evaluate_response(command_with_spaces, response_spaces, 2)
            log_result(log_file, f"{command} Test Multiple Spaces", command_with_spaces, response_spaces, result_spaces)
            print(f"{command} Test Multiple Spaces: {result_spaces} — {response_spaces.strip()[:80] or '[EMPTY RESPONSE]'}")

            # Special handling for STSLLT (without the pre/post logic)
            if command == "STSLLT":
                response_stsllt = send_command(ser, command)
                log_result(log_file, f"{command} Test", command, response_stsllt, "Tested Command")
                print(f"{command} Test: {response_stsllt.strip()[:80] or '[EMPTY RESPONSE]'}")

            # Determine if all 3 cases passed
            if result_valid == "PASS" and result_invalid == "PASS" and result_spaces == "PASS":
                working_count += 1
            else:
                not_working_count += 1

        log_file.write(f"\n=== Test Complete ===\n")
        log_file.write(f"Working commands: {working_count}\n")
        log_file.write(f"Not working commands: {not_working_count}\n")
        print(f"✅ All command tests completed. Working commands: {working_count}, Not working commands: {not_working_count}")
        log_file.flush()

    input("Press Enter to exit...")

# === Run the test ===
run_test()
