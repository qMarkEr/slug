using LibreHardwareMonitor.Hardware;
using System;
using System.Threading;
using System.Threading.Tasks;

public class HardwareService : IDisposable
{
    private readonly Computer _computer;
    private ISensor? _cpuSensor;
    private ISensor? _gpuSensor;

    private readonly object _lock = new();
    private float? _cpuTemp;
    private float? _gpuTemp;
    private readonly Timer _timer;

    public HardwareService()
    {
        _computer = new Computer
        {
            IsCpuEnabled = true,
            IsGpuEnabled = true
        };
        _computer.Open();

        // Find CPU and GPU sensors
        _cpuSensor = _computer.Hardware[0].Sensors[60];
        _gpuSensor = _computer.Hardware[1].Sensors[0];
        _computer.Hardware[0].Update();
        _computer.Hardware[1].Update();

        _timer = new Timer(UpdateTemps, null, 0, 1000);
    }

    private void UpdateTemps(object? state)
    {
        lock (_lock)
        {
            try
            {
                _cpuSensor?.Hardware.Update();
                _gpuSensor?.Hardware.Update();
                _cpuTemp = _cpuSensor?.Value;
                _gpuTemp = _gpuSensor?.Value;
            }
            catch { /* ignore */ }
        }
    }

    public (float? cpu, float? gpu) GetTemps()
    {
        lock (_lock)
        {
            return (_cpuTemp, _gpuTemp);
        }
    }

    public void Dispose()
    {
        _timer?.Dispose();
        _computer.Close();
    }
}
