import unittest
import pandas as pd
from scrape_stcp import scrape_stcp_data


class TestKalmanFilterAgainstSTCP(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        # Load Kalman Filter results
        cls.kf_results = pd.read_csv("../TestResults/kf_results.csv")
        cls.kf_results["bus_line"] = cls.kf_results["bus_line"].astype(str)
        cls.kf_results["stop_id"] = cls.kf_results["stop_id"].astype(str)

        # Load live data from STCP website
        cls.stcp_data = scrape_stcp_data("ISCP2")
        cls.stcp_data_dict = {
            entry["line"]: entry["wait_minutes"] for entry in cls.stcp_data
        }

        print("\n[INFO] Live STCP data fetched:")
        for line, wait in cls.stcp_data_dict.items():
            print(f" - Line {line}: {wait} min")

    def test_kalman_eta_versus_stcp(self):
        # Allow ±5 minute difference
        tolerance = 5

        for _, row in self.kf_results.iterrows():
            bus_line = row["bus_line"]
            eta = row["eta"]

            # Only test if STCP reported this bus line
            if bus_line not in self.stcp_data_dict:
                print(f"[SKIP] Bus line {bus_line} not found in live data.")
                continue

            stcp_wait = self.stcp_data_dict[bus_line]
            delta = abs(stcp_wait - eta)

            print(f"Testing line {bus_line}: KF ETA = {eta} min, STCP = {stcp_wait} min")

            self.assertLessEqual(
                delta,
                tolerance,
                msg=f"Line {bus_line}: KF ETA {eta} min differs from STCP {stcp_wait} min by more than {tolerance} min"
            )


if __name__ == "__main__":
    unittest.main()
