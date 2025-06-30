import unittest
import pandas as pd
from scrape_stcp import scrape_stcp_data
from collections import defaultdict


class TestKalmanFilterAgainstSTCP(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        # Ask the user to input a stop ID
        cls.stop_id = input("Enter stop ID (e.g., ISCP2): ").strip().upper()

        # Load Kalman Filter results
        cls.kf_results = pd.read_csv("../TestResults/kf_results.csv")
        cls.kf_results["bus_line"] = cls.kf_results["bus_line"].astype(str)
        cls.kf_results["stop_id"] = cls.kf_results["stop_id"].astype(str)

        # Filter KF results to just this stop
        cls.kf_results = cls.kf_results[cls.kf_results["stop_id"] == cls.stop_id.lower()]

        # Load live data from STCP website
        cls.stcp_data = scrape_stcp_data(cls.stop_id)

        # Group STCP ETAs by bus line
        cls.stcp_data_dict = defaultdict(list)
        for entry in cls.stcp_data:
            cls.stcp_data_dict[entry["line"]].append(entry["wait_minutes"])

        print(f"\n[INFO] Live STCP data fetched for stop '{cls.stop_id}':")
        for line, waits in cls.stcp_data_dict.items():
            print(f" - Line {line}: {waits} min")

    def test_kalman_eta_versus_stcp(self):
        # Allow ±5 minute difference
        tolerance = 10

        if self.kf_results.empty:
            self.skipTest(f"No Kalman results for stop '{self.stop_id}'")

        for _, row in self.kf_results.iterrows():
            bus_line = row["bus_line"]
            eta = row["eta"]

            if bus_line not in self.stcp_data_dict:
                print(f"[SKIP] Bus line {bus_line} not found in live data.")
                continue

            # Find the closest ETA from STCP for this line
            closest_stcp_eta = min(self.stcp_data_dict[bus_line], key=lambda x: abs(x - eta))
            delta = abs(closest_stcp_eta - eta)

            print(f"Testing line {bus_line}: KF ETA = {eta} min, Closest STCP ETA = {closest_stcp_eta} min")

            self.assertLessEqual(
                delta,
                tolerance,
                msg=f"Line {bus_line}: KF ETA {eta} min differs from closest STCP ETA {closest_stcp_eta} min by more than {tolerance} min"
            )


if __name__ == "__main__":
    unittest.main()
