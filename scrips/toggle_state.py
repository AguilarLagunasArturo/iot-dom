import requests
import json
import os

url = os.environ.get('IOT_SKILL_API')

get_data = {
	'action' : 'get-device-state',
	'device-id' : 'iot-skill-01'
}

set_data = {
	'action' : 'set-device-state',
	'device-id': 'iot-skill-01',
	'value' : '--'
}

toggle_data = {
	'action' : 'toggle-device-state',
	'device-id': 'iot-skill-01'
}

def set_state(state='OFF'):
	global set_data
	set_data['value'] = state
	return requests.post(url, json=set_data)

def get_state():
	global get_data
	return requests.post(url, json=get_data)

def toggle_state():
	global toggle_data
	return requests.post(url, json=toggle_data)

'''
def toggle_state():
	r = get_state()
	state_str = r.text.split(' ')[1][0:-1]
	state = 'OFF' if state_str == 'ON' else 'ON'
	r = set_state(state)
	return r
'''

r = toggle_state()

print(r.status_code)
print(r.url)
print(r.json())
