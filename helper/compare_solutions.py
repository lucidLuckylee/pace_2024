import pandas as pd
import argparse
import numpy as np

pd.set_option('display.max_columns', None)
pd.set_option('display.max_rows', None)


def main():

    parser = argparse.ArgumentParser(description="Run a program on files in a specified directory.")
    parser.add_argument("file_1", type=str, help="First file to compare")
    parser.add_argument("file_2", type=str, help="Second file to compare")

    args = parser.parse_args()

    file1 = pd.read_csv(args.file_1)
    file2 = pd.read_csv(args.file_2)

    diff = file1["crossings"] - file2["crossings"]
    
    procentuale_diff = file2["crossings"]/ file1["crossings"]
    mean = np.mean(procentuale_diff)
    
    print(procentuale_diff)
    positiv = np.sum(np.array(diff) > 0, axis=0)
    negative = np.sum(np.array(diff) <= 0, axis=0)

    print("File 1 got less or equal crossings "+ str(negative) + " times")
    print("File 1 got more crossings "+ str(positiv) + " times")
    print("File 1's solution is " + str(mean)+ " times better")



























if __name__== '__main__':
    main()