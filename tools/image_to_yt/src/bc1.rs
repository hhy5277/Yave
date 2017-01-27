
use std::cmp;
use image_data::{ImageData, Rgba, Block};

fn minf(a: f32, b: f32) -> f32 { if a < b { a } else { b } }
fn maxf(a: f32, b: f32) -> f32 { if a > b { a } else { b } }
fn saturate(a: f32) -> f32 { maxf(minf(a, 1.0), 0.0) }
fn fnorm(a: u8) -> f32 { a as f32 / 255.0 }
fn bnorm(a: f32) -> u8 { (saturate(a) * 255.0) as u8 }



fn pixel_dist(a: &Rgba, b: &Rgba) -> u32 {
	fn dist_one(a: u8, b: u8) -> u32 {
		let x = a as i32 - b as i32;
		(x * x) as u32
	}
	dist_one(a[0], b[0]) + 
	dist_one(a[1], b[1]) + 
	dist_one(a[2], b[2])
}

fn block_dist(table: &[Rgba; 4], pixels: &Block) -> u32 {
	let mut sum = 0u32;
	let mut max = 0u32;
	for pix in pixels.into_iter().rev() {
		let e = (0..4).into_iter().map(|x| pixel_dist(&table[x], pix)).min().unwrap();
		sum += e;
		max = cmp::max(max, e);
	}
	sum + max * 16
}







fn build_entry(ends: &(Rgba, Rgba), index: usize) -> Rgba {
	let coefs = match index {
		0x0 => (3, 0),
		0x1 => (0, 3),
		0x2 => (2 ,1),
		_ => (1, 2)
	};
	[((coefs.1 * ends.0[0] as u16 + coefs.0 * ends.1[0] as u16) / 3) as u8,
	 ((coefs.1 * ends.0[1] as u16 + coefs.0 * ends.1[1] as u16) / 3) as u8,
	 ((coefs.1 * ends.0[2] as u16 + coefs.0 * ends.1[2] as u16) / 3) as u8,
	 ((coefs.1 * ends.0[3] as u16 + coefs.0 * ends.1[3] as u16) / 3) as u8]
}

fn build_table(ends: &(Rgba, Rgba)) -> [Rgba; 4] {
	let mut interps = [[0u8; 4]; 4];
	for i in 0..4 {
		interps[i] = build_entry(ends, i);
	}
	interps
}

fn minmax(pixels: &Block) -> (Rgba, Rgba) {
	let mut min = [u8::max_value(); 4];
	let mut max = [0u8; 4];
	for p in pixels.into_iter() {
		for i in 0..4 {
			min[i] = cmp::min(min[i], p[i]);
			max[i] = cmp::max(max[i], p[i]);
		}
	}
	(min, max)
}


fn quantize_endpoints(ends: &(Rgba, Rgba)) -> (Rgba, Rgba) {
	fn quantize(end: &Rgba) -> Rgba {
		[end[0] & 0xF8, end[1] & 0xFC, end[2] & 0xF8, end[3] & 0x80]
	}
	(quantize(&ends.0), quantize(&ends.1))
}

fn extrapolate_endpoints(ends: &(Rgba, Rgba)) -> (Rgba, Rgba) {
	let mut a = ends.0;
	let mut b = ends.1;
	for i in 0..3 {
		let f = (fnorm(ends.0[i]) - fnorm(ends.1[i])) * 1.5;
		a[i] = bnorm(fnorm(ends.0[i]) - f);
		b[i] = bnorm(fnorm(ends.1[i]) + f);
	}
	(a, b)
}

fn sort_quantize_endpoints(ends: &(Rgba, Rgba)) -> (Rgba, Rgba) {
	if encode_endpoint(&ends.0) > encode_endpoint(&ends.1) {
		quantize_endpoints(&(ends.1, ends.0))
	} else {
		quantize_endpoints(&(ends.0, ends.1))
	}
}

fn build_endpoints(pixels: &Block, quality: u8) -> Vec<(Rgba, Rgba)> {
	let (min, max) = minmax(pixels);

	let mut out = Vec::new();
	out.push(quantize_endpoints(&(min, max)));

	if quality > 0 && min != max {
		for i in 0..15 {
			for j in (i + 1)..16 {
				let px = (pixels[i], pixels[j]);
				out.push(sort_quantize_endpoints(&px));
				if quality > 1 {
					out.push(sort_quantize_endpoints(&extrapolate_endpoints(&px)));
				}
			}
		}
	}
	out
}









fn encode_endpoint(end: &Rgba) -> u16 {
	((end[0] as u16 >> 3) << 11) |
	((end[1] as u16 >> 2) << 5) |
	(end[2] as u16 >> 3)
}

fn encode_pixels(table: &[Rgba; 4], pixels: &Block) -> u32 {
	let mut mask = 0u32;
	for pix in pixels.into_iter().rev() {
		let best = (0..4).into_iter().min_by_key(|x| pixel_dist(&table[*x], pix)).unwrap() as u32;
		mask = (mask << 2) | (best & 0x03);
	}
	mask
}

fn encode_block(pixels: &Block, quality: u8) -> (u64, u32) {
	let ((min, max), table) = build_endpoints(pixels, quality).iter()
		.map(|pts| (*pts, build_table(&(pts.0, pts.1))))
		.min_by_key(|tbl| block_dist(&tbl.1, pixels)).unwrap();

	let dist = block_dist(&table, pixels);

	let min = encode_endpoint(&min) as u64;
	let max = encode_endpoint(&max) as u64;

	let encoded = encode_pixels(&table, &pixels) as u64;

	((encoded << 32) | (min << 16) | max, dist)
}







pub fn encode(image: &ImageData, quality: u8) -> Result<Vec<u8>, ()> {
	if image.size.0 % 4 != 0 || image.size.1 % 4 != 0 {
		return Err(());
	}

    let mut out = Vec::with_capacity(image.pixels().count() / 8);
    let mut total_dist = 0u32;
   	
	for blk in image.blocks() {
		let (mut encoded, dist) = encode_block(&blk, quality);
		total_dist += dist;

		for _ in 0..8 {
			out.push((encoded & 0xFF) as u8);
			encoded >>= 8;
   		}
	}
	println!("per-pixel dist = {:?}", total_dist / image.pixels().count() as u32);
	Ok(out)
}
