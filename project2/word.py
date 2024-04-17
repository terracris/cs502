import re

def extract_function_calls(text):
    # Regular expression to match function calls
    function_call_pattern = re.compile(r'(\b\w+\()')

    # Find all matches in the text
    matches = function_call_pattern.findall(text)

    # Extract unique function names
    unique_functions = set(matches)

    return unique_functions

def count_unique_function_calls(file_path):
    try:
        with open(file_path, 'r') as file:
            # Read the content of the file
            content = file.read()

            # Extract unique function calls from the content
            unique_functions = extract_function_calls(content)

            # Print the result
            print(f"Number of unique function calls in the file '{file_path}': {len(unique_functions)}")
            for word in unique_functions:
                print(word)

    except FileNotFoundError:
        print(f"File not found: {file_path}")
    except Exception as e:
        print(f"An error occurred: {e}")

# Example usage:
file_path = 'ls.slog'  # Replace with the path to your file
count_unique_function_calls(file_path)
