//Writen by TjarkG and published under the MIT License

use std::fs;
use std::env;

extern crate chrono;
use chrono::prelude::DateTime;
use chrono::Utc;
use chrono::prelude::*;
use std::time::{SystemTime, UNIX_EPOCH};

mod tty;
mod reading;
pub use self::reading::Reading;

fn main()
{
	let args: Vec<String> = env::args().collect();
	let mut opend_uart = false;

	if env::args().len() == 1 || !(args[1].contains("COM") || args[1].contains("/dev"))
	{
		eprintln!("{}: first argument must be target Serial Port", args[0]);
	}
	else
	{
		tty::start(&args[1]);
		opend_uart = true;
	}

	for arg in args.iter()
	{
		if arg.contains("-h")	//looks for -h, --help and erverything in between
		{
			print_help();
			continue;
		}

		if arg == &args[0] || arg == &args[1]	//scipe the first two args
		{
			continue;
		}

		if opend_uart
		{
			if arg.contains("-rm")
			{
				set_command(String::from("CR"));
				print_csv_reading(get_reading());
			}
			else if arg.contains("-r")
			{
				set_command(String::from("CR"));
				print_reading(get_reading());
			}
			else if arg.contains("-s")
			{
				store_readings(arg.contains("-sc"));
			}
			else if arg.contains("-f")
			{
				//try syncing two times
				if ((sync_time() as i64 - (get_command("TG") as i64)).abs()) > 3
				{
					if ((sync_time() as i64 - (get_command("TG") as i64)).abs()) > 3
						{ eprintln!("Time Syncronization failed"); }
					else
						{ eprintln!("Syncronized System Times on second attempt"); }
				}
			}
			else if arg.contains("-i?")
			{
				println!("Messurment intervall: {}", get_command("IG"))
			}
			else if arg.contains("-i")
			{
				println!("test")
			}
			else if arg.contains("-t")
			{
				let err_codes = ["UART Transmittion", "AVCC", "RTC running", "RTC initialized", "Flash Signatur", "Flash erase", "Flash read/write", "UART Tx level",
				"UART Rx level", "Outside Temperatur", "Light Sensor", "Intervall set", "Temperatur offset set", "BME connected", "BME Readings in range"];

				let error = get_command("DR");

				for i in 0..err_codes.len()
				{
					let mut errstr = "Error";
					if error & (1 << i) == 0
						{ errstr = "Ok"; }
					
					println!("{:.24}{}", err_codes[i], errstr);
				}

				if error == 0 
            	    { println!("\nSelf Test passed"); }
            	else
            	    { println!("\nError detected, Code {}",error); }
			}
			else if arg.contains("-delete")
			{
				set_command(String::from("DEL"));
			}
			else if arg.contains("-ct")
			{
				let mut t_in = arg.chars();
				t_in.next();
				t_in.next();
				t_in.next();
				let t_in =  (t_in.as_str().parse::<f32>().unwrap()*5.0) as i16;

				set_command(String::from("CR"));
				let ip = get_reading();
				let off_old = get_command("OGT") as i16;

				let off = t_in - (ip.temperatur_out  as i16 -off_old+128) + 128;
				set_command(format!("OST{}", off));
				println!("Outside Temperatur set:{:.1}C Old Offset: {} New Offset:{} Offset Vertified: {}",t_in as f32/5.0, off_old-128, off-128, get_command("OGT")-128);
			}
			else if arg.contains("-gh")
			{
				println!("{}", get_command("GH") as u16)
			}
			else
			{
				eprintln!("Unknow Argument: {}", arg)
			}
		}
	}
}

fn get_command(input: &str) -> i32
{
	set_command(String::from(input));
	tty::get().parse::<i32>().unwrap()
}

fn set_command(input: String)
{
	tty::print(&input);
	let response = tty::get();
	if !response.contains(&input)
	{
		panic!("Error tranmitting UART Command {}: recieved {}", input, response);
	}
}

fn print_help()
{
	let filename = "../README.md";

	let contents = fs::read_to_string(filename)
        .expect("can't open Helpfile");

	println!("From {}:", filename);
    println!("{}", contents.replace("# ", "").replace("* ", ""));
}

fn get_reading() -> Reading		//return new reading or 42,0,0... for EOF
{
	let buf = tty::get();
	let mut out = Reading::default();
	
	if buf.contains("EOF")
	{
		out.time = 42;
		return out;
	}
	let split = buf.split(",");
	let vec: Vec<&str> = split.collect();
	out.time = 				vec[0].parse::<u32>().unwrap();
	out.light = 			vec[1].parse::<u16>().unwrap();
	out.temperatur_out =	vec[2].parse::<u8>().unwrap();
	out.temperatur_in = 	vec[3].parse::<u16>().unwrap();
	out.pressure = 			vec[4].parse::<u16>().unwrap();
	out.humidity_air = 		vec[5].parse::<u8>().unwrap();
	out.humidity_soil = 	vec[6].parse::<u8>().unwrap();
	out.iaq = 				vec[7].parse::<i16>().unwrap();
	return out;
}

fn store_readings(commenting: bool)
{
	if commenting 
        { eprintln!("Startet Saving Readings..."); }
	println!("UTC,Light,°C out,°C in,hPa,RH Air,RH Soil,IAQ");
	set_command(String::from("AR"));
	let mut ln_cnt = 0;
	loop
	{
		let ip = get_reading();
		if ip.time == 42
			{ break; }
		print_csv_reading(ip);

		if ln_cnt % 250 == 0 && commenting
			{ print!("#"); }
		ln_cnt += 1;
	}
	if commenting 
        { eprintln!("\nFinished! {} Readings Saved", ln_cnt); }
}

fn print_reading(ip: Reading)
{
	let datetime: DateTime<Utc>  = DateTime::from_utc(NaiveDateTime::from_timestamp(ip.time as i64, 0), Utc);
    let timestamp_str = datetime.format("%d.%m.%Y %H:%M:%S").to_string();

	print!("Current Reading: Time: {} UTC Outside: {}lux {:.1}C Inside: {:.1}C {}hPa",
		timestamp_str, ip.light, (ip.temperatur_out as f32/5.0), (ip.temperatur_in as f32/10.0), ip.pressure);

	if ip.humidity_air != 0 
		{ print!(" Air: {}%RH", ip.humidity_air); }
	if ip.humidity_soil != 0
		{ print!(" Soil: {}%RH", ip.humidity_soil); }
	if ip.iaq != 0
		{ print!(" {}IAQ", ip.iaq); }
	println!("");
}

fn print_csv_reading(ip: Reading)
{
	let datetime: DateTime<Utc>  = DateTime::from_utc(NaiveDateTime::from_timestamp(ip.time as i64, 0), Utc);
    let timestamp_str = datetime.format("%d.%m.%Y %H:%M:%S").to_string();

	println!("{},{},{:.1},{:.1},{},{},{},{}",
		timestamp_str, ip.light, (ip.temperatur_out as f32/5.0), (ip.temperatur_in as f32/10.0), ip.pressure, ip.humidity_air, ip.humidity_soil, ip.iaq);

}

fn sync_time() -> u64
{
	let rawtime: u64;
	match SystemTime::now().duration_since(UNIX_EPOCH)
	{
		Ok(n) => rawtime = n.as_secs(),
		Err(_) => panic!("SystemTime before UNIX EPOCH!"),
	}
	set_command(format!("TS{}", rawtime));
	return rawtime;
}