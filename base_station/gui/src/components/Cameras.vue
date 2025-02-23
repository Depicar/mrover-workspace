<template>
  <div class="wrap">
    <h3>Cameras</h3>
    <div>
      <Checkbox v-bind:name="'Microscope'" v-on:toggle="toggleMicroscopeCam()" ref="micro"/>
    </div>
    <div class="cameraselection">
      <CameraSelection class="cameraspace1" v-bind:cam_index="cam_index_1" v-on:cam_index="setCamIndex($event, 1)"/>
      <CameraSelection class="cameraspace2" v-bind:cam_index="cam_index_2" v-on:cam_index="setCamIndex($event, 2)"/>
    </div>
  </div>
</template>

<script>
  import CommIndicator from './CommIndicator.vue'
  import CameraSelection from './CameraSelection.vue'
  import Checkbox from "./Checkbox.vue"

  let interval;

  export default {
    data() {
      return {
        dual_stream: false,
        cam_index_1: -1,
        cam_index_2: -1,
        microscope_cam: false
      }
    },

    beforeDestroy: function () {
      window.clearInterval(interval);
    },

    created: function () {
      const JOYSTICK_CONFIG = {
        'down_left_button': 6,
        'up_left_button': 7,
        'down_middle_button': 8,
        'up_middle_button': 9,
        'down_right_button': 10,
        'up_right_button': 11
      }

      const CAMERA_NUM = {
        'down_left_button': 1,
        'up_left_button': 2,
        'down_middle_button': 3,
        'up_middle_button': 4,
        'down_right_button': 5,
        'up_right_button': 6
      }

      window.addEventListener('keydown', (e) => {
        const activeElement = document.activeElement;
        const inputs = ['input', 'select', 'textarea'];

        //Prevent camera change if inside text area
        if (activeElement && inputs.indexOf(activeElement.tagName.toLowerCase()) !== -1) {
            return;
        }

        if(e.keyCode>=49 && e.keyCode<=54)  //keys 1 to 6
          this.cam_index_1 = e.keyCode-48
      })

      // Change cam index based on joystick button
      interval = window.setInterval(() => {
        const gamepads = navigator.getGamepads()
        for (let i = 0; i < 2; i++) {
          const gamepad = gamepads[i]
          if (gamepad) {
            if (gamepad.id.includes('Logitech')) {
              if (gamepad.buttons[JOYSTICK_CONFIG['down_left_button']]['pressed']) {
                this.cam_index_1 = CAMERA_NUM['down_left_button']
              } else if (gamepad.buttons[JOYSTICK_CONFIG['up_left_button']]['pressed']) {
                this.cam_index_1 = CAMERA_NUM['up_left_button']
              } else if (gamepad.buttons[JOYSTICK_CONFIG['down_middle_button']]['pressed']) {
                this.cam_index_1 = CAMERA_NUM['down_middle_button']
              } else if (gamepad.buttons[JOYSTICK_CONFIG['up_middle_button']]['pressed']) {
                this.cam_index_1 = CAMERA_NUM['up_middle_button']
              } else if (gamepad.buttons[JOYSTICK_CONFIG['down_right_button']]['pressed']) {
                this.cam_index_1 = CAMERA_NUM['down_right_button']
              } else if (gamepad.buttons[JOYSTICK_CONFIG['up_right_button']]['pressed']) {
                this.cam_index_1 = CAMERA_NUM['up_right_button']
              }
            }
          }
        }
        this.$parent.publish('/microscope', {type: "Microscope", streaming: this.microscope_cam === true})
      }, 250)
    },

    methods: {
      setCamIndex: function (new_index, stream) {
        if (stream === 1 && (new_index !== this.cam_index_2 || new_index === -1)) {
          this.cam_index_1 = new_index
        } 
        else if (stream === 2 && (new_index !== this.cam_index_1 || new_index === -1)) {
          this.cam_index_2 = new_index
        }
      },

      toggleDualStream: function () {
        this.dual_stream = !this.dual_stream
        if (!this.dual_stream) {
          this.cams[this.cam_index_2] = false
          this.cam_index_2 = -1
        }
      },

      toggleMicroscopeCam: function () {
        this.microscope_cam = !this.microscope_cam
      },

      sendCameras: function() {
        this.$parent.publish("/cameras_cmd", {
          'type': 'Cameras',
          'port_0': this.cam_index_1,
          'port_1': this.cam_index_2,
        })
      }
    },

    watch: {
      cam_index_1() {
        this.sendCameras()
      },

      cam_index_2() {
        this.sendCameras()
      }
    },

    components: {
      CommIndicator,
      CameraSelection,
      Checkbox
    }
  }
</script>

<style>
  .wrap {
    display: grid;
    grid-gap: 10px;
    grid-template-columns: 1fr;
    grid-template-rows: 60px;
    grid-template-areas: "header";
    font-family: sans-serif;
    height: 100%;
  }

  .cameraselection {
    display: grid;
    grid-template-columns: 1fr 1fr;
    grid-template-areas: "cameraspace1 cameraspace2"
  }

  .cam_buttons {
    height:20px;
    width:100px;
  }

  .box {
    border-radius: 5px;
    padding: 10px;
    border: 1px solid black;
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
    display: flex;
  }

  ul#vitals li {
    display: inline;
    padding: 0px 10px 0px 0px;
  }
</style>
