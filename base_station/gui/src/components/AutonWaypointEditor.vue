<template>
  <div class="wrap">
    <div class="col-wrap" style="left: 0;">
      <div class="box">
        <div class="identification">
          Name: <input v-model="name" size="15">
          ID: <input v-model="id" type="number" max="249" min="-1" step="1">
          Gate Width: <input v-model="gate_width" type="number" max="3" min="2" step="1">
        </div>
        <br>
        <input type="radio" v-model="odom_format_in" value="D" class="checkbox"><font size="2">D</font>
        <input type="radio" v-model="odom_format_in" value="DM" class="checkbox"><font size="2">DM</font>
        <input type="radio" v-model="odom_format_in" value="DMS" class="checkbox"><font size="2">DMS</font><br>
        <div class="wp-input">
          <p><input v-model.number="input.lat.d" size="13">º</p>
          <p v-if="this.min_enabled"><input v-model.number="input.lat.m" size="13">'</p>
          <p  v-if="this.sec_enabled"><input v-model.number="input.lat.s" size="13">"</p>
          N
        </div>
        <div class="wp-input">
          <p><input v-model.number="input.lon.d" size="13">º</p>
          <p v-if="this.min_enabled"><input v-model.number="input.lon.m" size="13">'</p>
          <p  v-if="this.sec_enabled"><input v-model.number="input.lon.s" size="13">"</p>
          E
        </div>
        <br>
        <div style="display:inline-block">
          <button v-on:click="addWaypoint(input)">Add Waypoint</button>
          <button v-on:click="addWaypoint(formatted_odom)">Drop Waypoint</button>
        </div>
      </div>
      <div class="box1">
        <div class="all-waypoints">
          <h4 class="waypoint-headers">All Waypoints</h4>
          <button v-on:click="clearWaypoint">Clear Waypoints</button>
        </div>
        <draggable v-model="storedWaypoints" class="dragArea" draggable=".item'">
          <WaypointItem v-for="waypoint, i in storedWaypoints" :key="i" v-bind:waypoint="waypoint" v-bind:list="0" v-bind:index="i" v-on:delete="deleteItem($event)" v-on:toggleSearch="toggleSearch($event)" v-on:toggleGate="toggleGate($event)" v-on:add="addItem($event)"/>
        </draggable>
      </div>
    </div>
    <div class="col-wrap" style="left: 50%">
      <div class="box datagrid">
        <div class="autonmode">
          <Checkbox ref="checkbox" v-bind:name="autonButtonText" v-bind:color="autonButtonColor"  v-on:toggle="toggleAutonMode($event)"/>
        </div>
        <div class="stats">
          <p>
            Navigation State: {{nav_status.nav_state_name}}<br>
            Waypoints Traveled: {{nav_status.completed_wps}}/{{nav_status.total_wps}}<br>
          </p>
        </div>
        <div class="joystick light-bg">
          <AutonJoystickReading v-bind:AutonDriveControl="AutonDriveControl"/>
        </div>
      </div>
      <div class="box1">
        <h4 class="waypoint-headers">Current Course</h4>
        <draggable v-model="route" class="dragArea" draggable=".item'">
          <WaypointItem v-for="waypoint, i in route" :key="i" v-bind:waypoint="waypoint" v-bind:list="1" v-bind:index="i" v-bind:name="name" v-bind:id="id" v-on:delete="deleteItem($event)" v-on:toggleSearch="toggleSearch($event)" v-on:toggleGate="toggleGate($event)" v-on:add="addItem($event)"/>
        </draggable>
      </div>
    </div>
  </div>
</template>

<script>
import Checkbox from './AutonModeCheckbox.vue'
import draggable from 'vuedraggable'
import {convertDMS} from '../utils.js';
import AutonJoystickReading from './AutonJoystickReading.vue'
import WaypointItem from './AutonWaypointItem.vue'
import {mapMutations, mapGetters} from 'vuex'
import _ from 'lodash';
import fnvPlus from 'fnv-plus';
import L from 'leaflet'

let interval;

export default {

  props: {
    odom: {
      type: Object,
      required: true
    },
    AutonDriveControl: {
      type: Object,
      required: true
    }
  },

  data () {
    return {
      name: "Waypoint",
      id: "-1",
      gate_width: "3",
      odom_format_in: 'DM',
      input: {
        lat: {
          d: 0,
          m: 0,
          s: 0
        },
        lon: {
          d: 0,
          m: 0,
          s: 0
        }
      },

      nav_status: {
        nav_state_name: "Off",
        completed_wps: 0,
        total_wps: 0
      },

      storedWaypoints: [],
      route: [],

      autonButtonColor: "red",
      waitingForNav: false

    }
  },

  beforeDestroy: function () {
    window.clearInterval(interval);
  },

  created: function () {

    this.$parent.subscribe('/nav_status', (msg) => {
      this.nav_status = msg
      if(this.waitingForNav){
        if(this.nav_status.nav_state_name!="Off" && this.autonEnabled){
          this.waitingForNav = false
          this.autonButtonColor = "green"
        }
        else if(this.nav_status.nav_state_name=="Off" && !this.autonEnabled){
          this.waitingForNav = false
          this.autonButtonColor = "red"
        }
      }
    })

    interval = window.setInterval(() => {

      this.$parent.publish('/auton', {type: 'AutonState', is_auton: this.autonEnabled})

      let course = {
        num_waypoints: this.route.length,
        waypoints: _.map(this.route, (waypoint) => {
          const lat = waypoint.lat.d + waypoint.lat.m/60 + waypoint.lat.s/3600;
          const lon = waypoint.lon.d + waypoint.lon.m/60 + waypoint.lon.s/3600;
          const latitude_deg = Math.trunc(lat);
          const longitude_deg = Math.trunc(lon);

          return {
            type: "Waypoint",
            search: waypoint.search,
            gate: waypoint.gate,
            gate_width: parseFloat(waypoint.gate_width),
            id: parseFloat(waypoint.id),
            odom: {
              latitude_deg: latitude_deg,
              latitude_min: (lat - latitude_deg) * 60,
              longitude_deg: longitude_deg,
              longitude_min: (lon - longitude_deg) * 60,
              bearing_deg: 0,
              speed: -1,
              type: "Odometry"
            },
          }
        })
      };
      course.hash = fnvPlus.fast1a52(JSON.stringify(course));
      course.type = 'Course'
      this.$parent.publish('/course', course)
    }, 100);
  },

  methods: {
    ...mapMutations('autonomy',{
      setRoute: 'setRoute',
      setWaypointList: 'setWaypointList',
      setAutonMode: 'setAutonMode',
      setOdomFormat: 'setOdomFormat'
    }),

    deleteItem: function (payload) {
      if(payload.list === 0) {
        this.storedWaypoints.splice(payload.index, 1)
      } else if(payload.list === 1) {
        this.route.splice(payload.index, 1)
      }
    },

    // Add item from all waypoints div to current waypoints div
    addItem: function (payload) {
       if(payload.list === 0) {
        this.route.push(this.storedWaypoints[payload.index])
      } else if(payload.list === 1) {
        this.storedWaypoints.push(this.route[payload.index])
      }
    },

    toggleSearch: function (payload) {
      if(payload.list === 0) {
        this.storedWaypoints[payload.index].search = !this.storedWaypoints[payload.index].search
      } else if(payload.list === 1) {
        this.route[payload.index].search = !this.route[payload.index].search
      }
    },

    toggleGate: function (payload) {
      if(payload.list === 0) {
        this.storedWaypoints[payload.index].gate = !this.storedWaypoints[payload.index].gate
      } else if(payload.list === 1) {
        this.route[payload.index].gate = !this.route[payload.index].gate
      }
    },

    addWaypoint: function (coord) {
      this.storedWaypoints.push({
        name: this.name,
        id: this.id,
        lat: Object.assign({}, coord.lat),
        lon: Object.assign({}, coord.lon),
        search: false,
        gate: false,
        gate_width: this.gate_width
      });
    },

    clearWaypoint: function () {
      this.storedWaypoints = [];
    },

    toggleAutonMode: function (val) {
      this.setAutonMode(val)
      this.autonButtonColor = "yellow"
      this.waitingForNav = true;
    }
  },

  watch: {
    route: function (newRoute) {
      const waypoints = newRoute.map((waypoint) => {
        const lat = waypoint.lat.d + waypoint.lat.m/60 + waypoint.lat.s/3600;
        const lon = waypoint.lon.d + waypoint.lon.m/60 + waypoint.lon.s/3600;
        return { latLng: L.latLng(lat, lon), name: waypoint.name };
      });
      this.setRoute(waypoints);
    },

    storedWaypoints: function (newList) {
      const waypoints = newList.map((waypoint) => {
        const lat = waypoint.lat.d + waypoint.lat.m/60 + waypoint.lat.s/3600;
        const lon = waypoint.lon.d + waypoint.lon.m/60 + waypoint.lon.s/3600;
        return { latLng: L.latLng(lat, lon), name: waypoint.name };
      });
      this.setWaypointList(waypoints);
    },

    odom_format_in: function (newOdomFormat) {
      this.setOdomFormat(newOdomFormat);
      this.input.lat = convertDMS(this.input.lat, newOdomFormat);
      this.input.lon = convertDMS(this.input.lon, newOdomFormat);
      this.storedWaypoints.map((waypoint) => {
        waypoint.lat = convertDMS(waypoint.lat, newOdomFormat);
        waypoint.lon = convertDMS(waypoint.lon, newOdomFormat);
        return waypoint;
      });
      this.route.map((waypoint) => {
        waypoint.lat = convertDMS(waypoint.lat, newOdomFormat);
        waypoint.lon = convertDMS(waypoint.lon, newOdomFormat);
        return waypoint;
      });
    },

    clickPoint: function (newClickPoint){
      this.input.lat.d = newClickPoint.lat
      this.input.lon.d = newClickPoint.lon
      this.input.lat.m = 0;
      this.input.lon.m = 0;
      this.input.lat.s = 0;
      this.input.lon.s = 0;
      this.input.lat = convertDMS(this.input.lat, this.odom_format_in);
      this.input.lon = convertDMS(this.input.lon, this.odom_format_in);
    }
  },

  computed: {
    ...mapGetters('autonomy', {
      autonEnabled: 'autonEnabled',
      odom_format: 'odomFormat',
      clickPoint: "clickPoint"
    }),

    formatted_odom: function() {
      return {
        lat: convertDMS({d: this.odom.latitude_deg, m: this.odom.latitude_min, s: 0}, this.odom_format),
        lon: convertDMS({d: this.odom.longitude_deg, m: this.odom.longitude_min, s: 0}, this.odom_format)
      }
    },

    min_enabled: function() {
      return this.odom_format != 'D';
    },

    sec_enabled: function() {
      return this.odom_format == 'DMS';
    },

    autonButtonText: function() {
      return (this.autonButtonColor == "yellow") ? "Setting to "+this.autonEnabled : "Autonomy Mode"
    }

  },

  components: {
    draggable,
    WaypointItem,
    Checkbox,
    AutonJoystickReading
  }

}
</script>

<style scoped>

  .wrap {
    position: relative;
    display: flex;
    flex-direction: row;
    height: 100%;
    margin: auto;
  }

  .col-wrap {
    position: absolute;
    margin: 1.5px;
    display: inline-block;
    /*flex-direction: column; */
    height: 100%;
    width: 49.5%;
  }
  
  .col-wrap span{
    margin:-30px 0px 0px 0px;
  }

  .dragArea {
    height: 100%;
  }

  .identification{
    display: inline-block;
  }

  .box {
    border-radius: 5px;
    padding: 10px;
    border: 1px solid black;
    min-height: min-content;
    max-height: 32%;
    overflow: none;
    margin-bottom: 6px;
  }

  .box1 {
    border-radius: 5px;
    padding: 0px 5px 0px 5px;
    border: 1px solid black;
    overflow: scroll;
    min-height: min-content;
    max-height: 63%;
  }

  .datagrid {
      display: grid;
      grid-gap: 3px;
      grid-template-columns: 1fr 1fr;
      grid-template-rows: 1fr 1fr 0.25fr;
      grid-template-areas: "autonmode stats"
                           "toggle__button stats"
                           "joystick stats";
      font-family: sans-serif;
      min-height: min-content;
  }

  .all-waypoints{
    display: inline-flex;
  }

  .all-waypoints button{
    margin: 5px;
    width: 115px;
    height: 20px;
  }

  .autonmode{
    align-content: center;
  }

  .stats{
    grid-area: stats;
  }
  
  .joystick{
    grid-area: joystick;
  }
  
  .odom{
    grid-area: odom;
  }
  .wp-input p {
    display: inline;
  }

  .waypoint-headers{
    margin: 5px 0px 0px 5px;
  }

</style>
