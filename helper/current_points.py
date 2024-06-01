import argparse
import time

from bs4 import BeautifulSoup
from selenium.webdriver.chrome.options import Options
from selenium.webdriver.chrome.service import Service as ChromeService
from selenium import webdriver
from selenium.webdriver.common.by import By


def main(global_leaderboard, ignore_instance_40, username, password):
    chrome_options = Options()
    chrome_options.add_argument('--headless')
    chrome_options.add_argument('--disable-gpu')

    service = ChromeService()
    driver = webdriver.Chrome(service=service, options=chrome_options)

    if global_leaderboard:
        driver.get('https://www.optil.io/optilion/problem/3212#tab-4')
    else:
        driver.get('https://www.optil.io/optilion/login')
        time.sleep(1)
        driver.find_element(value="username").send_keys(username)
        driver.find_element(value="password").send_keys(password)
        driver.find_element(by=By.XPATH,
                            value="/html/body/div[2]/section/div/div/div/div[2]/form/div[4]/div[1]/button").click()
        time.sleep(5)
        driver.get('https://www.optil.io/optilion/problem/3212#tab-3')

    time.sleep(5)
    rendered_html = driver.page_source
    driver.quit()

    soup = BeautifulSoup(rendered_html, 'html.parser')
    table = soup.find('table').find("tbody")

    points = {}
    for row in table.find_all('tr'):
        elements = row.find_all('td')
        name = elements[0].get_text()
        points[name] = []

        if global_leaderboard:
            elements = elements[4:]
        else:
            elements = elements[6:]

        for i, score in enumerate(elements):

            if i == 39 and ignore_instance_40:
                continue

            try:
                points[name].append(int(score.get_text(strip=True).split('.')[0].replace(",", "")))
            except:
                points[name].append("Error")

    best_scores = []

    for i in range(0, len(list(points.values())[0])):
        best_score = 1000000000000000
        for value in points.values():
            if isinstance(value[i], int) and value[i] < best_score:
                best_score = value[i]
        best_scores.append(best_score)

    overall_scores = []

    for name in points:
        overall_points = 0
        for i in range(0, len(best_scores)):
            if isinstance(points[name][i], int):
                score = points[name][i]
                best_score = best_scores[i]

                if score == 0:
                    overall_points += 1
                else:
                    overall_points += best_score / score
        overall_scores.append((name, overall_points))

    overall_scores.sort(key=lambda x: x[1], reverse=True)
    for name, score in overall_scores:
        print(name, score)


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--global_leaderboard', type=bool,
                        help='Weather to use the global leaderboard (leaderboard is not hidden) or only my runs')
    parser.add_argument('--ignore_instance_40', type=bool,
                        help='Weather to ignore instance 40 (this instance seems to be the hardest and introduce a high bias)')

    parser.add_argument("--username", type=str, default="", help="Username to login (needed for my runs)")
    parser.add_argument("--password", type=str, default="", help="Password to login (needed for my runs)")

    args = parser.parse_args()

    main(args.global_leaderboard, args.ignore_instance_40, args.username, args.password)
