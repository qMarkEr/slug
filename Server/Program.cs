using Microsoft.AspNetCore.Builder;
using Microsoft.Extensions.DependencyInjection;
using Microsoft.Extensions.Hosting;

var builder = WebApplication.CreateBuilder(args);

// 👇 Allow it to run as a Windows Service
builder.Host.UseWindowsService(options =>
{
    options.ServiceName = "HotSlug"; // service name shown in Services.msc
});

// Force Kestrel to listen on all network interfaces (LAN access)
builder.WebHost.ConfigureKestrel(serverOptions =>
{
    serverOptions.ListenAnyIP(5221);
});

builder.Services.AddSingleton<HardwareService>();
builder.Services.AddControllers();

var app = builder.Build();

app.UseDefaultFiles();
app.UseStaticFiles();
app.MapControllers();

app.Run();



//   |
//   |
//   |

//  _
//  _|
// |_

//  _
//  _|
//  _|

// |_|
//   |

//  _
// |_
//  _|

//  _
// |_
// |_|

//  _
//   |
//   |

//  _
// |_|
// |_|

//  _
// |_|
//  _|

//  _
// | |
// |_|