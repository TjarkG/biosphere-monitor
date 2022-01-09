//Writen by TjarkG and published under the MIT License

use std::io;

pub fn start(port: &String)
{
    println!("UART: Started on {}", port);
}

pub fn print(input: &String)
{
    println!("UART: {}", input);
}

pub fn get() -> String
{
    let mut out = String::new();

    io::stdin()
		.read_line(&mut out)
        .expect("Failed to read line");

	out.pop();
    return out;
}

//Example Reading:
// 23463457,2,23,23,947,25,10,0