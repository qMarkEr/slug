using Microsoft.AspNetCore.Mvc;
using System.Runtime.InteropServices;

namespace TempMonitorWeb.Controllers
{
    
    [ApiController]
    [Route("api/[controller]")]
    public class TempsController : ControllerBase
    {

        private readonly HardwareService _hardwareService;
        public TempsController(HardwareService service) => _hardwareService = service;

        [HttpGet]
        public IActionResult GetTemps()
        {
            var (cpu, gpu) = _hardwareService.GetTemps();
            return Ok(new { cpu, gpu });
        }
    }

    [ApiController]
    [Route("api/[controller]")]
    public class ControlController : ControllerBase
    {
        // WinAPI call to sleep the system
        [DllImport("PowrProf.dll", SetLastError = true)]
        private static extern bool SetSuspendState(bool hibernate, bool forceCritical, bool disableWakeEvent);

        [HttpPost("sleep")]
        public IActionResult Sleep()
        {
            try
            {
                Task.Run(() =>
                {
                    // Give HTTP response time to be sent
                    System.Threading.Thread.Sleep(100);
                    SetSuspendState(false, false, false);
                });
                return Ok(new { status = "sleeping" });
            }
            catch (Exception ex)
            {
                return StatusCode(500, new { error = ex.Message });
            }
        }
    }
}
