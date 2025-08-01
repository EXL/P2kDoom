package com.sfprod.jwadutil;

public record Color(int r, int g, int b) {

	int getRGB() {
		return 0xff000000 | (r << 16) | (g << 8) | b;
	}

	double gray() {
		return r * 0.299 + g * 0.587 + b * 0.114;
	}

	int calculateDistance(Color that) {
		int distr = this.r - that.r;
		int distg = this.g - that.g;
		int distb = this.b - that.b;

		return distr * distr + distg * distg + distb * distb;
	}

	Color blendColors(Color that) {
		int rBlended = (int) Math.sqrt((this.r * this.r + that.r * that.r) / 2);
		int gBlended = (int) Math.sqrt((this.g * this.g + that.g * that.g) / 2);
		int bBlended = (int) Math.sqrt((this.b * this.b + that.b * that.b) / 2);
		return new Color(rBlended, gBlended, bBlended);
	}
}
