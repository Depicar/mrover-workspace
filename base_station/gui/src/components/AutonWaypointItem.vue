<template>
  <div class="waypoint-item">
    <div class="identification">
      <p>{{waypoint.name}}, ID: {{waypoint.id}} Gate Width: {{waypoint.gate ? waypoint.gate_width : 'n/a'}}</p>
    </div>
    <div class="buttons">
      <button class="red" v-on:click="$emit('add', {'list': list, 'index': index})">Add</button>
      <button v-bind:class="[waypoint.search ? 'green' : 'red']" v-on:click="$emit('toggleSearch', {'list': list, 'index': index})">Search</button>
      <button v-bind:class="[waypoint.gate ? 'green' : 'red']" v-on:click="$emit('toggleGate', {'list': list, 'index': index})">Gate</button>
      <button class="red" v-on:click="$emit('delete', {'list': list, 'index': index})">Delete</button>
    </div>
    <div class="location">
      <div>
        <p>{{waypoint.lat.d}}º</p>
        <p v-if="this.min_enabled">{{waypoint.lat.m}}'</p>
        <p  v-if="this.sec_enabled">{{waypoint.lat.s}}"</p>
        N <b>&nbsp;|</b> 
        <p>{{waypoint.lon.d}}º</p>
        <p v-if="this.min_enabled">{{waypoint.lon.m}}'</p>
        <p  v-if="this.sec_enabled">{{waypoint.lon.s}}"</p>
        W
      </div>
    </div>
  </div>
</template>

<script>
import {mapGetters} from 'vuex';

export default {

  props: {
    waypoint: {
      type: Object,
      required: true
    },

    list: {
      type: Number,
      required: true
    },

    index: {
      type: Number,
      required: true
    }
  },

  computed: {
    ...mapGetters('autonomy', {
      odom_format: 'odomFormat'
    }),

    min_enabled: function() {
      return this.odom_format != 'D';
    },

    sec_enabled: function() {
      return this.odom_format == 'DMS';
    }
  },
}
</script>

<style scoped>
  .waypoint-item {
    background-color: rgb(180, 180, 180);
    border-radius: 5px;
    padding: 1px 10px 10px 10px;
    border: 1px solid black;

    margin: 5px;
  }
  .location {
    grid-area: location;
  }

  .location p {
    display: inline;
  }

  .buttons {
    grid-area: buttons;
    align-self: center;
    justify-self: center;
    display: block;
  }

  .red {
    background-color: red;
  }

  .green {
    background-color: green;
  }

  button {
    width: auto;
    height: auto;
    padding: 7px;
    font-weight: bold;
  }

  p {
    margin: 5px
  }
</style>
