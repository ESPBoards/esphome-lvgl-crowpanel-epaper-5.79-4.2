# Projects

Complete builds rather than starting points. The configs in `examples/` show the
minimum needed to drive a panel; these are finished things you can flash.

## Weather dashboard - 5.79"

![Weather dashboard on CrowPanel 5.79" E-Paper](../../docs/owm.jpeg)

[`project_epaper_5p79_owm_api.yaml`](project_epaper_5p79_owm_api.yaml) - pulls
the forecast from the OpenWeatherMap API and lays it out with LVGL. Partial
refresh keeps the redraws quick; call `id(epd).force_full_update()` to clear
ghosting.

```
esphome run examples/projects/project_epaper_5p79_owm_api.yaml
```

### Secrets

Needs a `secrets.yaml` in this directory - ESPHome looks for it beside the
config file and does not fall back to the parent. Either put one here, or
symlink the one in `examples/`:

```
ln -s ../secrets.yaml examples/projects/secrets.yaml
```

Beyond the usual `wifi_ssid` / `wifi_password` / `api_key` / `ota_password`, it
needs your OpenWeatherMap details:

```yaml
owm_api_key: "..."
owm_lat: "54.6872"
owm_lon: "25.2797"
```
