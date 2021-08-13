# -*- coding: utf-8 -*-

# Copyright 2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
#
# Licensed under the Amazon Software License (the "License"). You may not use this file except in
# compliance with the License. A copy of the License is located at
#
#    http://aws.amazon.com/asl/
#
# or in the "license" file accompanying this file. This file is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, express or implied. See the License for the specific
# language governing permissions and limitations under the License.

import boto3
import json
import random
import uuid
import time
from datetime import datetime, timedelta

aws_dynamodb = boto3.client('dynamodb')

def lambda_handler(request, context):

    # Dump the request for logging - check the CloudWatch logs
    print('lambda_handler request  -----')
    print(json.dumps(request))

    if context is not None:
        print('lambda_handler context  -----')
        print(context)

    # Validate we have an Alexa directive
    if 'directive' not in request:
        aer = AlexaResponse(
            name='ErrorResponse',
            payload={'type': 'INVALID_DIRECTIVE',
                     'message': 'Missing key: directive, Is the request a valid Alexa Directive?'})
        return send_response(aer.get())

    # Check the payload version
    payload_version = request['directive']['header']['payloadVersion']
    if payload_version != '3':
        aer = AlexaResponse(
            name='ErrorResponse',
            payload={'type': 'INTERNAL_ERROR',
                     'message': 'This skill only supports Smart Home API version 3'})
        return send_response(aer.get())

    # Crack open the request and see what is being requested
    name = request['directive']['header']['name']
    namespace = request['directive']['header']['namespace']

    # Handle the incoming request from Alexa based on the namespace

    if namespace == 'Alexa.Authorization':
        if name == 'AcceptGrant':
            # Note: This sample accepts any grant request
            # In your implementation you would use the code and token to get and store access tokens
            grant_code = request['directive']['payload']['grant']['code']
            grantee_token = request['directive']['payload']['grantee']['token']
            aar = AlexaResponse(namespace='Alexa.Authorization', name='AcceptGrant.Response')
            return send_response(aar.get())

    if namespace == 'Alexa.Discovery':
        if name == 'Discover':
            device_uuid = get_latest_alexa_available()
            if device_uuid:
                register_alexa_device(device_uuid)
                adr = AlexaResponse(namespace='Alexa.Discovery', name='Discover.Response')
                capability_alexa = adr.create_payload_endpoint_capability()
                capability_alexa_powercontroller = adr.create_payload_endpoint_capability(
                    interface='Alexa.PowerController',
                    supported=[{'name': 'powerState'}],
                    proactively_reported=True,
                    retrievable=True)
                adr.add_payload_endpoint(
                    description='Este es el mejor prototipo de un dispositivo IoT de todos los tiempos',
                    friendly_name='switch-{}'.format(device_uuid),
                    endpoint_id=device_uuid,
                    manufacturer_name='SS2021',
                    capabilities=[capability_alexa, capability_alexa_powercontroller])
                return send_response(adr.get())
            else:
                return AlexaResponse(
                    name='ErrorResponse',
                    payload={'type': 'ENDPOINT_UNREACHABLE', 'message': 'UUID in database unavailable.'}).get()

    if namespace == 'Alexa.PowerController':
        # Note: This sample always returns a success response for either a request to TurnOff or TurnOn
        endpoint_id = request['directive']['endpoint']['endpointId']
        power_state_value = 'OFF' if name == 'TurnOff' else 'ON'
        correlation_token = request['directive']['header']['correlationToken']

        # Get device last connection
        response = get_device_data(endpoint_id)
        last_connection = response['Item']['lastConnection']['S']

        # Check for an error when device is not responsive
        if is_time_exceeded(last_connection):
            return AlexaResponse(
                name='ErrorResponse',
                payload={'type': 'ENDPOINT_UNREACHABLE', 'message': 'Unable to reach endpoint database.'}).get()

        # Check for an error when setting the state
        state_set = set_device_state(endpoint_id=endpoint_id, state='powerState', value=power_state_value)
        if not state_set:
            return AlexaResponse(
                name='ErrorResponse',
                payload={'type': 'ENDPOINT_UNREACHABLE', 'message': 'Unable to reach endpoint database.'}).get()

        apcr = AlexaResponse(correlation_token=correlation_token)
        apcr.add_context_property(namespace='Alexa.PowerController', name='powerState', value=power_state_value)


        return send_response(apcr.get())

    if namespace == 'Alexa':
        if name == 'ReportState':
            endpoint_id = request['directive']['endpoint']['endpointId']
            correlation_token = request['directive']['header']['correlationToken']
            token = request['directive']['endpoint']['scope']['token']

            response = get_device_data(endpoint_id)
            state = response['Item']['powerStateValue']['S']
            last_connection = response['Item']['lastConnection']['S']

            if is_time_exceeded(last_connection):
                arr = AlexaResponse(namespace='Alexa', name='StateReport', correlation_token=correlation_token, endpoint_id=endpoint_id, token=token)
                arr.add_context_property(namespace='Alexa.PowerController', name='powerState', value=state)
                arr.add_context_property(value={'value': 'UNREACHABLE'})
                return send_response(arr.get())
            else:
                arr = AlexaResponse(namespace='Alexa', name='StateReport', correlation_token=correlation_token, endpoint_id=endpoint_id, token=token)
                arr.add_context_property(namespace='Alexa.PowerController', name='powerState', value=state)
                arr.add_context_property(value={'value': 'OK'})
                return send_response(arr.get())

def send_response(response):
    print('lambda_handler response -----')
    print(json.dumps(response))
    return response

def set_device_state(endpoint_id, state, value):
    attribute_key = state + 'Value'
    response = aws_dynamodb.update_item(
        TableName='IotSkill',
        Key={'ItemId': {'S': endpoint_id}},
        AttributeUpdates={attribute_key: {'Action': 'PUT', 'Value': {'S': value}}})
    print(response)
    if response['ResponseMetadata']['HTTPStatusCode'] == 200:
        return True
    else:
        return False

def register_alexa_device(endpoint_id):
    # DEBUG: CHECK FOR ERROR IN DATABASE AND SEND ALEXA ErrorResponse
    response = aws_dynamodb.update_item(
        TableName='IotSkill',
        Key={'ItemId': {'S': endpoint_id}},
        AttributeUpdates={'alexaState': {'Action': 'PUT', 'Value': {'S': 'ON'}}})
    print(response)
    if response['ResponseMetadata']['HTTPStatusCode'] == 200:
        return True
    else:
        return False

def get_device_data(device_id='iot-skill-01'):
    # DEBUG: CHECK FOR ERROR IN DATABASE AND SEND ALEXA ErrorResponse
    # DEBUG: Alexa.EndpointHealth", value=OK or value=UNREACHABLE
    response = aws_dynamodb.get_item(
        TableName='IotSkill',
        Key={'ItemId': {'S': device_id}})
    print(response)
    return response

def get_utc_timestamp(seconds=None):
    return time.strftime('%Y-%m-%dT%H:%M:%S.00Z', time.gmtime(seconds))

def is_time_exceeded(ts, dt=10):
    t_current = time.strptime(get_utc_timestamp(), '%Y-%m-%dT%H:%M:%S.00Z')
    t_old = time.strptime(ts, '%Y-%m-%dT%H:%M:%S.00Z')

    td1 = datetime.fromtimestamp(time.mktime(t_current)) - datetime.fromtimestamp(time.mktime(t_old))
    td2 = timedelta(seconds=dt)

    print('[td1]', td1)
    print('[td2]', td1)
    print('[td1 > td2]', td1 > td2)

    return td1 > td2

def get_latest_alexa_available():
    response = aws_dynamodb.scan(TableName='IotSkill')
    items = response['Items']
    devices = []
    while 'LastEvaluatedKey' in response:
        print(response['LastEvaluatedKey'])
        response = table.scan(ExclusiveStartKey=response['LastEvaluatedKey'])
        items.extend(response['Items'])
    for i in items:
        if i.get('alexaState'):
            if i.get('alexaState').get('S') == 'OFF':
                devices.append(i.get('ItemId').get('S'))
    if devices:
        return devices[-1]
    else:
        return None

class AlexaResponse:

    def __init__(self, **kwargs):

        self.context_properties = []
        self.payload_endpoints = []

        # Set up the response structure
        self.context = {}
        self.event = {
            'header': {
                'namespace': kwargs.get('namespace', 'Alexa'),
                'name': kwargs.get('name', 'Response'),
                'messageId': str(uuid.uuid4()),
                'payloadVersion': kwargs.get('payload_version', '3')
                # 'correlation_token': kwargs.get('correlation_token', 'INVALID')
            },
            'endpoint': {
                "scope": {
                    "type": "BearerToken",
                    "token": kwargs.get('token', 'INVALID')
                },
                "endpointId": kwargs.get('endpoint_id', 'INVALID')
            },
            'payload': kwargs.get('payload', {})
        }

        if 'correlation_token' in kwargs:
            self.event['header']['correlation_token'] = kwargs.get('correlation_token', 'INVALID')

        if 'cookie' in kwargs:
            self.event['endpoint']['cookie'] = kwargs.get('cookie', '{}')

        # No endpoint in an AcceptGrant or Discover request
        if self.event['header']['name'] == 'AcceptGrant.Response' or self.event['header']['name'] == 'Discover.Response':
            self.event.pop('endpoint')

    def add_context_property(self, **kwargs):
        self.context_properties.append(self.create_context_property(**kwargs))

    def add_cookie(self, key, value):

        if "cookies" in self is None:
            self.cookies = {}

        self.cookies[key] = value

    def add_payload_endpoint(self, **kwargs):
        self.payload_endpoints.append(self.create_payload_endpoint(**kwargs))

    def create_context_property(self, **kwargs):
        return {
            'namespace': kwargs.get('namespace', 'Alexa.EndpointHealth'),
            'name': kwargs.get('name', 'connectivity'),
            'value': kwargs.get('value', {'value': 'OK'}),
            'timeOfSample': get_utc_timestamp(),
            'uncertaintyInMilliseconds': kwargs.get('uncertainty_in_milliseconds', 0)
        }

    def create_payload_endpoint(self, **kwargs):
        # Return the proper structure expected for the endpoint
        endpoint = {
            'capabilities': kwargs.get('capabilities', []),
            'description': kwargs.get('description', 'Sample Endpoint Description'),
            'displayCategories': kwargs.get('display_categories', ['SMARTPLUG']),
            'endpointId': kwargs.get('endpoint_id', 'endpoint_' + "%0.6d" % random.randint(0, 999999)),
            'friendlyName': kwargs.get('friendly_name', 'Sample Endpoint'),
            'manufacturerName': kwargs.get('manufacturer_name', 'Sample Manufacturer')
        }

        if 'cookie' in kwargs:
            endpoint['cookie'] = kwargs.get('cookie', {})

        return endpoint

    def create_payload_endpoint_capability(self, **kwargs):
        capability = {
            'type': kwargs.get('type', 'AlexaInterface'),
            'interface': kwargs.get('interface', 'Alexa'),
            'version': kwargs.get('version', '3')
        }
        supported = kwargs.get('supported', None)
        if supported:
            capability['properties'] = {}
            capability['properties']['supported'] = supported
            capability['properties']['proactivelyReported'] = kwargs.get('proactively_reported', False)
            capability['properties']['retrievable'] = kwargs.get('retrievable', False)
        return capability

    def get(self, remove_empty=True):

        response = {
            'context': self.context,
            'event': self.event
        }

        if len(self.context_properties) > 0:
            response['context']['properties'] = self.context_properties

        if len(self.payload_endpoints) > 0:
            response['event']['payload']['endpoints'] = self.payload_endpoints

        if remove_empty:
            if len(response['context']) < 1:
                response.pop('context')

        return response

    def set_payload(self, payload):
        self.event['payload'] = payload

    def set_payload_endpoint(self, payload_endpoints):
        self.payload_endpoints = payload_endpoints

    def set_payload_endpoints(self, payload_endpoints):
        if 'endpoints' not in self.event['payload']:
            self.event['payload']['endpoints'] = []

        self.event['payload']['endpoints'] = payload_endpoints
