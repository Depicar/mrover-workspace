<template>
  <div class="wrapper">
    <div class="box header">
      <img src="/static/mrover.png" alt="MRover" title="MRover" width="48" height="48" />
      <h1>ERD Dashboard</h1>
      <div class="spacer"></div>
      <div class="comms">
        <ul id="vitals">
          <li><CommIndicator v-bind:connected="connections.websocket" name="Web Socket" /></li>
          <li><CommIndicator v-bind:connected="connections.lcm" name="Rover Connection Status" /></li>
        </ul>
      </div>
      <div class="spacer"></div>
      <div class="help">
        <img src="/static/help.png" alt="Help" title="Help" width="48" height="48" />
      </div>
      <div class="helpscreen"></div>
      <div class="helpimages" style="display: flex; align-items: center; justify-content: space-evenly">
        <img v-if="controlMode === 'arm'" src="/static/arm.png" alt="Robot Arm" title="Robot Arm Controls" style="width: auto; height: 70%; display: inline-block" />
        <img v-else-if="controlMode === 'soil_ac'" src="/static/soil_ac.png" alt="Soil Acquisition" title="Soil Acquisition Controls" style="width: auto; height: 70%; display: inline-block" />
        <img src="/static/joystick.png" alt="Joystick" title="Joystick Controls" style="width: auto; height: 70%; display: inline-block" />
      </div>
    </div>

    <div class="box odom light-bg">
      <OdometryReading v-bind:odom="odom"/>
    </div>
    <div class="box cameras light-bg">
      <Cameras/>
    </div>
    <div class="box ik-controls light-bg">
      <IKControls/>
    </div>
    <div class="box map light-bg">
      <RoverMap v-bind:odom="odom"/>
    </div>
    <div class="box controls light-bg">
      <ArmControls/>
    </div>
    <div class="box encoder light-bg">
      <EncoderCounts/>
    </div>
    <div class="box drive light-bg">
      <DriveControls/>
    </div>
    <div class="box waypoint-editor light-bg">
      <ERDWaypointEditor/>
    </div>
    <div class ="box pdb light-bg">
      <PDBFuse/>
    </div>
    <div class="box drive-motor light-bg">
      <DriveVelDataV/>
    </div>
    <div class="spacer"></div>
  </div>
</template>

<script>
import { mapGetters } from 'vuex'
import Cameras from './Cameras.vue'
import IKControls from './IKControls.vue'
import RoverMap from './ERDRoverMap.vue'
import CommIndicator from './CommIndicator.vue'
import OdometryReading from './OdometryReading.vue'
import ArmControls from './ArmControls.vue'
import DriveControls from './DriveControls.vue'
import EncoderCounts from './EncoderCounts.vue'
import LCMBridge from 'lcm_bridge_client/dist/bridge.js'
import PDBFuse from './PDBFuse.vue'
import DriveVelDataV from './DriveVelDataV.vue' 
import ERDWaypointEditor from './ERDWaypointEditor.vue'

export default {
  name: 'RATask',
  data () {
    return {
      lcm_: null,

      odom: {
        latitude_deg: 38,
        latitude_min: 24.38226,
        longitude_deg: -110,
        longitude_min: -47.51724,
        bearing_deg: 0,
        speed: 0
      },

      connections: {
        websocket: false,
        lcm: false
      },

      nav_status: {
        completed_wps: 0,
        total_wps: 0
      }
    }
  },

  methods: {
    publish: function (channel, payload) {
      this.lcm_.publish(channel, payload)
    },

    subscribe: function (channel, callbackFn) {
      if( (typeof callbackFn !== "function") || (callbackFn.length !== 1)) {
        console.error("Callback Function is invalid (should take 1 parameter)")
      }
      this.lcm_.subscribe(channel, callbackFn)
    }
  },

  computed: {
    ...mapGetters('controls', {
      controlMode: 'controlMode'
    }),
  },

  created: function () {
    this.lcm_ = new LCMBridge(
      'ws://localhost:8001',
      // Update WebSocket connection state
      (online) => {
        this.lcm_.setHomePage()
        this.connections.websocket = online
      },
      // Update connection states
      (online) => {
        this.connections.lcm = online[0]
      },
      // Subscribed LCM message received
      (msg) => {
        if (msg.topic === '/odometry') {
          this.odom = msg.message
        } else if (msg.topic === '/debugMessage') {
          if (msg['message']['isError']) {
            console.error(msg['message']['message'])
          } else {
            console.log(msg['message']['message'])
          }
        }
      },
      // Subscriptions
      [
        {'topic': '/odometry', 'type': 'Odometry'},
        {'topic': '/sensors', 'type': 'Sensors'},
        {'topic': '/temperature', 'type': 'Temperature'},
        {'topic': '/ra_offset_pos', 'type': 'RAPosition'},
        {'topic': '/arm_control_state_to_gui', 'type': 'ArmControlState'},
        {'topic': '/debugMessage', 'type': 'DebugMessage'},
        {'topic': '/drive_vel_data', 'type': 'DriveVelData'},
        {'topic': '/drive_state_data', 'type': 'DriveStateData'},
        {'topic': '/ik_reset', 'type': 'Signal'}
      ]
    )
  },

  components: {
    RoverMap,
    Cameras,
    CommIndicator,
    ArmControls,
    DriveControls,
    EncoderCounts,
    OdometryReading,
    IKControls,
    PDBFuse,
    DriveVelDataV,
    ERDWaypointEditor
  }
}
</script>

<style scoped>
  .wrapper {
    display: grid;
    grid-gap: 10px;
    grid-template-columns: 1fr 1.2fr;
    grid-template-rows: 60px auto auto auto auto auto auto auto;
    grid-template-areas: "header header"
                         "map cameras"
                         "map ik-controls"
                         "waypoint-editor ik-controls"
                         "encoder controls"
                         "odom drive"
                         "pdb drive-motor";
    font-family: sans-serif;
    height: auto;
  }

  .box {
    border-radius: 5px;
    padding: 10px;
    border: 1px solid black;
  }

  .light-bg {
    background-color: LightGrey;
  }

  img {
    border: none;
    border-radius: 0px;
  }

  .header {
    grid-area: header;
    display: flex;
    align-items: center;
  }

  .header h1 {
    margin-left: 5px;
  }

  .spacer {
    flex-grow: 0.8;
  }

  .comms {
    display: flex;
    flex-direction: column;
    align-items: flex-start;
  }

  .comms * {
    margin-top: 2px;
    margin-bottom: 2px;
  }

  .helpscreen {
    z-index: 1000000000;
    display: block;
    visibility: hidden;
    background-color: black;
    opacity: 0.8;
    position: absolute;
    left: 0px;
    top: 0px;
    width: 100%;
    height: 100%;
  }

  .helpimages {
    z-index: 1000000001;
    visibility: hidden;
    position: absolute;
    left: 5%;
    top: 5%;
    width: 90%;
    height: 90%;
  }

  .help {
    z-index: 1000000002;
    display: flex;
    float: right;
    opacity: 0.8;
    cursor: auto;
  }

  .help:hover {
    opacity: 1.0;
    cursor: pointer;
  }

  .help:hover ~ .helpscreen, .help:hover ~ .helpimages {
    visibility: visible;
  }

  .odom {
    grid-area: odom;
    font-size: 1em;
  }

  .diags {
    grid-area: diags;
  }

  .map {
    grid-area: map;
  }

  .ik-controls {
    grid-area: ik-controls;
  }

  .controls {
    grid-area: controls;
    font-size: 1em;
    display: flex;
  }

  .drive-motor {
    grid-area: drive-motor;
  }

  .waypoint-editor {
    grid-area: waypoint-editor
  }

  .pdb {
    grid-area: pdb;
  }

  .drive {
    grid-area: drive;
  }

  .new-select {
    display: inline-block;
  }

  .fil-hori-now {
    margin-top: 20px;
  }

  .new-select {
    display: inline-block;
  }

  .fil-hori-now {
    margin-top: 20px;
  }

  ul#vitals li {
    display: inline;
    float: left;
    padding: 0px 10px 0px 0px;
  }
</style>
