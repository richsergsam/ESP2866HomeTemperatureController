const timerId = setInterval(tick, 1000);

const current_params_url = "/current_params";

// дожидаемся загрузки документа и назначаем события на кнопки
document.addEventListener("DOMContentLoaded", function () {
  document.getElementById("mode_24").addEventListener("click", mode_24);
  document.getElementById("mode_12").addEventListener("click", mode_12);


  // DECREMENT AND INCREMENT BUTTONS

  // temperature always (24)
  set_dec_inc_listeners("dec_t_24", "inc_t_24", "temperature_24");

  // temperature day
  set_dec_inc_listeners("dec_t_day", "inc_t_day", "temperature_day");

  // temperature night
  set_dec_inc_listeners("dec_t_night", "inc_t_night", "temperature_night");

  // temperature delta
  set_dec_inc_listeners("dec_t_delta", "inc_t_delta", "temperature_delta");

  document.getElementById("submit_button").addEventListener("click", setParams);
});

// first page initialisation
fetch(current_params_url)
  .then((response) => {
    return response.json();
  })
  .then((data) => {
    updateLabels(data);
    updatePage(data);
    document.getElementById("root").classList.remove("blur");
  });

function tick() {
  console.log("timer tick");
  getParams();
}

function getParams() {
  fetch(current_params_url)
    .then((response) => {
      return response.json();
    })
    .then((data) => {
      updateLabels(data);
    });
}

function updateLabels(params) {
  //current time
  let c_time = params["current_time"];
  document.getElementById("current_time").innerHTML =
    c_time[0] + ":" + c_time[1] + ":" + c_time[2];

  //current temperature
  let temperature = parseFloat(params["t_current"]).toFixed(1)
  document.getElementById("current_temperature").innerHTML =
  temperature + " c&deg;";

  //current relay status
  let pump = document.getElementById("pump");
  if (params["relay_state"] == 1) {
    pump.classList.add("pump_on");
    pump.innerText = "Обогрев ON";
  } else {
    pump.classList.remove("pump_on");
    pump.innerText = "Обогрев OFF";
  }
}

function updatePage(params) {
  // current mode
  if (params["mode"] == 24) {
    mode_24();
  }
  if (params["mode"] == 12) {
    mode_12();
  }

  // temperature 24
  document.getElementById("temperature_24").value = params["t_always"];

  // temperature day
  document.getElementById("temperature_day").value = params["t_day"];

  // temperature night
  document.getElementById("temperature_night").value = params["t_night"];

  // temperature delta
  document.getElementById("temperature_delta").value = params["t_delta"];

  //day start time
  let dst = params["day_start_time"];
  document.getElementById("time_day_start").value = dst[0] + ":" + dst[1];

  //day end time
  let det = params["day_end_time"];
  document.getElementById("time_day_end").value = det[0] + ":" + det[1];
}

function mode_12() {
  document.getElementById("mode_12").classList.add("enable");
  document.getElementById("mode_24").classList.remove("enable");
  document.getElementById("temperature_24_container").style.display = "none";
  document
    .getElementById("temperature_day_container")
    .style.removeProperty("display");
  document
    .getElementById("temperature_night_container")
    .style.removeProperty("display");
  document
    .getElementById("time_day_start_container")
    .style.removeProperty("display");
  document
    .getElementById("time_day_end_container")
    .style.removeProperty("display");
}

function mode_24() {
  document.getElementById("mode_24").classList.add("enable");
  document.getElementById("mode_12").classList.remove("enable");
  document
    .getElementById("temperature_24_container")
    .style.removeProperty("display");
  document.getElementById("temperature_day_container").style.display = "none";
  document.getElementById("temperature_night_container").style.display = "none";
  document.getElementById("time_day_start_container").style.display = "none";
  document.getElementById("time_day_end_container").style.display = "none";
}

function set_dec_inc_listeners(dec_button_id, inc_button_id, input_id) {
  document.getElementById(dec_button_id).addEventListener("click", () => {
    dec(input_id);
    round_on_change(input_id);
  });
  document.getElementById(inc_button_id).addEventListener("click", () => {
    inc(input_id);
    round_on_change(input_id);
  });
}

function dec(element_name) {
  let t = document.getElementById(element_name);
  t.value = parseFloat(t.value) - parseFloat(t.step);
}

function inc(element_name) {
  let t = document.getElementById(element_name);
  t.value = parseFloat(t.value) + parseFloat(t.step);
}

function setParams() {
  let mode = 0;
  if (document.getElementById("mode_24").classList.contains("enable")) {
    mode = 24;
  } else {
    mode = 12;
  }

  let data = {
    t_delta: parseFloat(document.getElementById("temperature_delta").value),
    t_always: parseFloat(document.getElementById("temperature_24").value),
    t_day: parseFloat(document.getElementById("temperature_day").value),
    t_night: parseFloat(document.getElementById("temperature_night").value),
    mode: mode,
    day_start_time: time_input_to_json(
      document.getElementById("time_day_start").value
    ),
    day_end_time: time_input_to_json(
      document.getElementById("time_day_end").value
    ),
  };

  //alert(JSON.stringify(data, null, 4));
  postData(current_params_url, data)
    .then((response) => {
      console.log(response); // JSON data parsed by `response.json()` call

      if (response["success"] == true) {
        alert_show();
      } else {
        alert(response["error_msg"]);
      }
    })
    .catch((error) => {
      alert("Response error: " + error);
    });
}

async function postData(url = "", data = {}) {
  // Default options are marked with *
  const response = await fetch(url, {
    method: "POST", // *GET, POST, PUT, DELETE, etc.
    //mode: "cors", // no-cors, *cors, same-origin
    //cache: "no-cache", // *default, no-cache, reload, force-cache, only-if-cached
    //credentials: "same-origin", // include, *same-origin, omit
    headers: {
      "Content-Type": "application/json",
      // 'Content-Type': 'application/x-www-form-urlencoded',
    },
    //redirect: "follow", // manual, *follow, error
    //referrerPolicy: "no-referrer", // no-referrer, *client
    body: JSON.stringify(data), // body data type must match "Content-Type" header
  });
  return await response.json(); // parses JSON response into native JavaScript objects
}

function time_input_to_json(string) {
  let arr = string.split(":");
  return [arr[0], arr[1], "00"];
}

function alert_hide() {
  document.getElementById("alert").style.display = "none";
  document.getElementById("root").classList.remove("blur");
}

function alert_show() {
  document.getElementById("alert").style.removeProperty("display");
  document.getElementById("root").classList.add("blur");
}


function round_on_change(element_id){
  let el = document.getElementById(element_id);
  el.value = parseFloat(el.value).toFixed(1);
}