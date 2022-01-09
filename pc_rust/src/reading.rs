//Writen by TjarkG and published under the MIT License
//struct to store Sensor redings in on PC and MC

#[derive(Debug, Default)]
pub struct Reading
{
    pub time: u32,       //UNIX Timestamp of reading beginning
    pub light: u16,             //outside illuminance       in lux
    pub temperatur_out: u8,     //outside Temperatur        in °C*5
    pub temperatur_in: u16,     //inside Temperatur         in °C*10
    pub pressure: u16,          //inside Pressur            in hPa
    pub humidity_air: u8,       //inside relativ humidity   in %
    pub humidity_soil: u8,      //inside soil humidity      in %, 0 without a Sensor
    pub iaq: i16,               //Air Quality Index         in IAQ, 0 without Sensor
}