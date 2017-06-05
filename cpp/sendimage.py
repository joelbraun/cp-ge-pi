import requests
url = 'https://cp-parkix-configure.run.aws-usw02-pr.ice.predix.io/api/sensor/68690372-5B3C-4B5D-AF51-E80A2E1CEF56/image'
files = {'file': open('output.jpg', 'rb')}
req = requests.put(url, files=files)
print req
