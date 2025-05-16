from selenium import webdriver
from selenium.webdriver.chrome.options import Options
from bs4 import BeautifulSoup
import re
import time

def scrape_stcp_data(stop):
    url = "https://www.stcp.pt/pt/viajar/horarios/?paragem="+ stop +"&t=smsbus"

    # Setup headless Chrome
    options = Options()
    options.headless = True
    options.add_argument('--no-sandbox')
    options.add_argument('--disable-dev-shm-usage')

    driver = webdriver.Chrome(options=options)
    driver.get(url)

    time.sleep(5)  # Allow JS to load

    soup = BeautifulSoup(driver.page_source, 'html.parser')
    driver.quit()

    results = []

    table = soup.find('table', {'id': 'smsBusResults'})
    if table:
        rows = table.find_all('tr')[1:]
        for row in rows:
            cols = row.find_all('td')
            if len(cols) >= 3:
                full_line = cols[0].get_text(strip=True)
                match = re.match(r"(\d+)", full_line)
                line = match.group(1) if match else full_line  # fallback to full_line if match fails

                time_str = cols[1].get_text(strip=True)
                wait_str = cols[2].get_text(strip=True)

                wait_minutes = int(re.sub(r'\D', '', wait_str)) if wait_str else -1

                results.append({
                    'line': line,
                    'time': time_str,
                    'wait_minutes': wait_minutes
                })

    return results

# Example usage
if __name__ == "__main__":
    data = scrape_stcp_data("ISCP2")
    for entry in data:
        print(f"Line: {entry['line']} - Time: {entry['time']} - Wait: {entry['wait_minutes']} min")
