import machine
import micropython
import utime
import urequests
import network
import gc, uos
"""
Variables
--------------------------------------------------------------------------------
"""
_PKEYS = ['hop','h','b0','b1','b2','b3','b4','b5','b6','b7','b8','b9','rssi','lqi','nxt','cnt']
_DATABYTES = ['b0','b1','b2','b3','b4','b5','b6','b7','b8','b9']
_HEADERS = [2,5,7,8,9,10,14]
# Network credentials pulled from inet.conf residing on the FS
# options check
micropython.mem_info(1)

def read_network_config():
    config_dict = {}
    try:
        with open('inet.conf', 'r') as conf_handler:
            for item in conf_handler.readlines():
                option = item.split("=")[0].strip()
                if len(item.split("=")) == 2 and '#' not in option:
                    value = item.split("=")[1].strip()
                    config_dict.update({option: value})
                else:
                    if '#' in option:
                        print(b"COMMENT in config")
                    else:
                        print(b"WARNING: Fucked up option, make it better")
    except Exception as e:
        print(b"WARNING: Errors in INFRA config, still going for AP")
        return False
    print(config_dict)
    return config_dict
gc.collect()
micropython.mem_info(1)

def set_up_infra(_SSID, _PASS, _TIMEOUT):
    sta = network.WLAN(network.STA_IF)
    ap = network.WLAN(network.AP_IF)
    print(b"Disabling AP")
    ap.active(False)
    print(b"Activating INFRA")
    sta.active(True)
    sta.isconnected() # False, it should be # Comments by Yoda
    print(b"Connecting to infra")
    sta.connect(_SSID, _PASS)
    print(b"Let's wait for the network to come up") # Hopefully not indefinitely
    while not (sta.isconnected()):
        if _TIMEOUT > 0:
            print(b"Trying... {} more times".format(_TIMEOUT))
            utime.sleep_ms(500)
            _TIMEOUT = _TIMEOUT - 1
        else:
            print(b"Out of retrys while waiting for network, going to sleep")
            utime.sleep_ms(500)
            return False
    network_config = sta.ifconfig()
    return network_config
gc.collect()
micropython.mem_info(1)

def send_to_influx(host, port, db, user, password, wind, measurement, name, value, tags):
    post = "http://{}:{}/write?db={}".format(host, port, db)
    serial_console.write(b"SENDING TO: " + post.encode())
    serial_console.write(b"\r\n")
    for tag in tags.keys():
        measurement = measurement + ',' + tag + '=' + tags[tag]
    data = "{_measure} {_name}={_value}\n wind,type=speed value={_speed}\n wind,type=direction value={_direction}".format(
        _measure=measurement,
        _name=name,
        _value=value,
        _speed=wind['speed'],
        _direction=wind['direction']
    )
    serial_console.write(b"POST_DATA: " + data.encode())
    serial_console.write(b"\r\n")
    try:
        return urequests.post(post, data=data)
    except Exception as e:
        serial_console.write(b"ERROR sending data to influx: " + str(result).encode())
        serial_console.write(b"ERROR sending data to influx: " + str(e).encode())
        serial_console.write(b"\r\n")
        return False

def packet_check(packet, header):
    packet_headers_ok = False
    packet_data_check = False
    if type(packet) == type(dict()):
        #serial_console.write(b"Is dict")
        if header in _HEADERS:
            #serial_console.write(b"Header OK")
            for pkey in _PKEYS:
                if pkey in packet.keys():
                    #serial_console.write(b"PKey in PKEYS")
                    packet_headers_ok = True
                    if pkey in _DATABYTES:
                        if packet[pkey] < 256:
                            packet_data_check = True
                        else:
                            serial_console.write(b"ERROR: {} > 255 ({})\r\n".format(pkey, packet[pkey]))
                            return False
                else:
                    serial_console.write(b"ERROR: {} not present in packet\r\n".formt(pkey))
                    return False
        else:
            serial_console.write(b"ERROR: Unknown header\r\n")
            return False
    else:
        serial_console.write(b"ERROR: packet not dict()\r\n")
        return False
    return packet_headers_ok and packet_data_check


gc.collect()
micropython.mem_info(1)


class davisDecoder(object):
    def __init__(self):
        __name__ = u'Davis value decoder class'


    def byte_split(self, data):
        msb = data >> 4
        lsb = data & 0b00001111
        result = {"MSB": msb, "LSB": lsb}
        return result

    def davis_id(self, header):
        bin_header = self.byte_split(header)
        unit_id = bin_header['LSB'] & 0b0111
        battery_low = bin_header['LSB'] >> 3
        davis_packet_id = bin_header['MSB']
        result = {"davis_id": unit_id,
                  "packet_id": davis_packet_id,
                  "bat_low": battery_low}
        return result

    def decode_wind(self, databytes):
        # wind speed in mph, i suppose. Let's convert it
        wind_speed = round(float(databytes['windspeed'] * 1.60934), 1)
        wind_direction_factor = round(float(360)/float(255), 1)
        wind_direction = databytes['winddir']
        wind_direction = float(wind_direction) * wind_direction_factor
        result = {"speed": wind_speed, "direction": wind_direction}
        return result

    def decode_temp(self, temp):
        temp_f = (float(temp)) / float(160) # in Fahrenheit
        temp_c = round((temp_f - 32) * float(5)/float(9), 1)
        result = {"celsius": temp_c, "fahrenheit": temp_f}
        return result

    def decode_humidity(self, hum):
        pass

    def supercap_decode(self, byte2, byte3):
        cap = (byte2 << 2) + (byte3 >> 6)
        result = float(cap / 100.00)
        return result

    def solarvolt_decode(self, byte2, byte3):
        solar = (byte2 << 1) + (byte3 >> 7)
        result = float(solar)
        return result

    def rain_decode(self, rain):
        result = float(rain & 0x7F)
        return result

    def rainrate_decode(self, byte2, byte3):
        # if byte3(b2 here) is 0xFF, or 255, there is no rain
        #print("b2:{} b3:{} = result:{}".format(byte2, byte3, byte2 + (byte3 >> 4 << 8)))
        if byte2 == 255:
            rainstate = 0
            rainrate = 0
        elif byte2 == 254:
            rainstate = 1
            rainrate = 0.2
        else:
            rainstate = 2
            if byte3 > 4:
                rainrate = 720 / ((byte3 >> 4 << 8) + byte2)
            else:
                rainrate = 0

        result = {"state": float(rainstate), "rate": float(rainrate)}
        #print(result)
        return result

    def DecodePacket(self, packet):
        self.wind = self.decode_wind(
            {"windspeed": received['b0'], "winddir": received['b1']})
        if davis_packet_id == 2:
            # SuperCap charge 0x2
            #serial_console.write(b'SCAP:')
            supercap = dd.supercap_decode(
                received['b2'], received['b3']
            )
            #serial_console.write(b"{}\r\n".format(supercap))
            self.influx_db = 'status'
            self.measurement = 'iss'
            self.name = 'voltage'
            self.tags = {'type': 'capacitor'}
            self.value = supercap
        elif davis_packet_id == 3:
            # No fucking idea 0x3
            pass
        elif davis_packet_id == 5:
            # Rainrate 0x5
            #serial_console.write(b'RAINRATE:')
            rainrate_dict = dd.rainrate_decode(
                received['b2'],
                received['b3']
            )
            #serial_console.write(b"{}\r\n".format(rainrate_dict))
            self.measurement = 'rain'
            self.name = 'value'
            self.tags = {'type': 'rainrate'}
            self.value = rainrate_dict['rate']
        elif davis_packet_id == 6:
            # Sun Irradiation 0x6 (NOT ON vantage Vue)
            pass
        elif davis_packet_id == 7:
            # Super Cap voltage 0x7
            #serial_console.write(b'SOLV:')
            solarvolt = dd.solarvolt_decode(
                received['b2'], received['b3']
            )
            #serial_console.write(b"{}\r\n".format(solarvolt))
            self.influx_db = 'status'
            self.measurement = 'iss'
            self.name = 'voltage'
            self.tags = {'type': 'solar'}
            self.value = solarvolt
        elif davis_packet_id == 8:
            # Temperature 0x8
            #serial_console.write(b'TEMP:')
            raw_temp = (received['b2'] << 8) + received['b3']
            temp_dict = dd.decode_temp(raw_temp)
            temp = float(temp_dict['celsius'])
            #serial_console.write(b"{}\r\n".format(temp))
            self.measurement = 'temphumi'
            self.name = 'temperature'
            self.tags = {'type': 'external'}
            self.value = temp
        elif davis_packet_id == 9:
            # Wind gusts 0x9
            #serial_console.write(b'WINDGUST:')
            windgust = received['b2'] * 1.60934
            #serial_console.write(b"{}\r\n".format(windgust))
            self.measurement = 'wind'
            self.name = 'value'
            self.tags = {'type': 'windgust'}
            self.value = windgust
        elif davis_packet_id == 10:
            # Humidity 0xa
            #serial_console.write(b'HUMI:')
            raw_humidity = (((received['b3'] >> 4) & 0b0011) << 8) \
                         + received['b2']
            humidity = round(int(raw_humidity) / float(10), 1)
            #serial_console.write(b"{}\r\n".format(humidity))
            self.measurement = 'temphumi'
            self.name = 'humidity'
            self.tags = {'type': 'external'}
            self.value = humidity
        elif davis_packet_id == 14:
            # Rain bucket tips 0xe
            #serial_console.write(b'RAINCOUNT:')
            raw_rain = (received['b2']) + (received['b3'] >> 7 << 8)
            rain = dd.rain_decode(raw_rain)
            #serial_console.write(b"{}\r\n".format(rain))
            self.measurement = 'rain'
            self.name = 'value'
            self.tags = {'type': 'rain_bucket_tips'}
            self.value = rain

#print('Console set to 115200')
"""
Network configuration section
--------------------------------------------------------------------------------
"""
micropython.mem_info(1)
gc.collect()
try:
    net_config = read_network_config()
    if net_config:
        net_conf_result = set_up_infra(net_config["ssid"], net_config["pass"], 15)
    else:
        print(b"ERROR: cannot read from inet.conf file")
    utime.sleep_ms(100)
    if net_conf_result:
        print("Connected:")
        print(b"  IP: {}".format(net_conf_result[0]))
        print(b"  MASK: {}".format(net_conf_result[1]))
        print(b"  GW: {}".format(net_conf_result[2]))
        print(b"  DNS: {}\n".format(net_conf_result[3]))
    else:
        print("WARNING: Network config not done")
except Exception as e:
    print(b"ERROR: Network configuration failed with: {}".format(e))
print(b"Network section done")
'''
--------------------------------------------------------------------------------
'''
micropython.mem_info(1)
gc.collect()
utime.sleep_ms(5000)
uos.dupterm(None, 1)
serial_console = machine.UART(0, baudrate=9600, timeout=1113, rxbuf=32)
dd = davisDecoder()

while True:
    thing = serial_console.read()
    if thing is not None:
        serial_console.write(thing)
        try:
            received = eval(thing.decode())
            header = dd.davis_id(received['h'])
        except Exception as e:
            serial_console.write(b'Failed_decode\r\n')
            header = None
        if header is not None:
            dd.wind = False
            dd.measurement = False
            dd.name = False
            dd.value = False
            dd.tags = False
            dd.influx_db = net_config['_INFLUX_DB']
            davis_unit_id = header['davis_id']
            davis_packet_id = header['packet_id']
            iss_battery_low = header['bat_low']
            if packet_check(received, davis_packet_id):
                serial_console.write(b'ID: {}, '.format(davis_unit_id))
                serial_console.write(b'PKT: {}, '.format(davis_packet_id))
                serial_console.write(b'B_LOW: {}'.format(iss_battery_low))
                serial_console.write(b':\r\n')
                dd.DecodePacket(received)
                serial_console.write(b'W_SPD: {}, '.format(dd.wind['speed']))
                serial_console.write(b'W_DIR: {}\r\n'.format(dd.wind['direction']))

                # send data to influx here
                if net_conf_result:
                    serial_console.write(b'NETCONF: {}:{}\r\n'.format(net_config['_INFLUX_HOST'], net_config['_INFLUX_PORT']))
                    serial_console.write(b' WIND:{}\r\n'.format(dd.wind))
                    serial_console.write(b' MEASUREMENT: {}\r\n'.format(dd.measurement))
                    serial_console.write(b' MEASURE NAME: {}\r\n'.format(dd.name))
                    serial_console.write(b' VALUE: {}\r\n'.format(dd.value))
                    serial_console.write(b' TAGS: {}\r\n'.format(dd.tags))
                    data_sent = False
                    try:
                        data_sent = send_to_influx(
                            net_config['_INFLUX_HOST'],
                            net_config['_INFLUX_PORT'],
                            dd.influx_db,
                            net_config['_INFLUX_USER'],
                            net_config['_INFLUX_PASS'],
                            dd.wind,
                            dd.measurement,
                            dd.name,
                            dd.value,
                            dd.tags
                            )
                    except Exception as e:
                        serial_console.write(b'ERROR: Data send \'urequest\': {}'.format(e))
                        serial_console.write(b'\r\n')
                    if data_sent != False:
                        serial_console.write(b"DATA SEND: {}".format(data_sent.status_code))
                        serial_console.write(b'\r\n')
                    else:
                        serial_console.write(b"DATA SEND FAIL")
                        serial_console.write(b'\r\n')

                else:
                    serial_console.write(b'WARNING: Not sending data, infra is not available')
                    serial_console.write(b'\r\n')
            else:
                serial_console.write(b'ERROR: Integrity check failed')
                serial_console.write(b'\r\n')
    else:
        if thing is None:
            serial_console.write(b'.')
        else:
            serial_console.write(thing)
            serial_console.write(b':\r\n')
    gc.collect()

uos.dupterm(machine.UART(0, 115200), 1)
print(b'Console set to 115200')
machine.UART(0, 115200)

