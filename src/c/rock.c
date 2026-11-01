#include "rock.h"

static int16_t isin(int deg) {
  while (deg < 0) deg += 360;
  deg %= 360;
  int q = deg % 180;
  int v = (q < 90) ? q * 255 / 90 : (180 - q) * 255 / 90;
  return (deg >= 180) ? (int16_t)(-v) : (int16_t)v;
}

static int16_t icos(int deg) {
  return isin(deg + 90);
}

static GPoint rot(int x, int y, int cx, int cy, int angle_deg) {
  int16_t s = isin(angle_deg);
  int16_t c = icos(angle_deg);
  int rx = (x * c - y * s) / 255;
  int ry = (x * s + y * c) / 255;
  return GPoint(cx + rx, cy + ry);
}

static uint32_t mix(uint32_t seed, uint32_t i) {
  seed ^= i * 0x9E3779B9u;
  seed *= 0x85EBCA6Bu;
  return seed ^ (seed >> 13);
}

static void gem_colors(uint8_t type, GColor *fill, GColor *hi, GColor *a, GColor *b) {
#ifdef PBL_COLOR
  switch (type) {
    case GEM_SAPPHIRE:
      *fill = GColorBlueMoon; *hi = GColorPictonBlue;
      *a = GColorDukeBlue; *b = GColorVividCerulean;
      break;
    case GEM_EMERALD:
      *fill = GColorIslamicGreen; *hi = GColorSpringBud;
      *a = GColorDarkGreen; *b = GColorMayGreen;
      break;
    case GEM_AMETHYST:
      *fill = GColorPurple; *hi = GColorShockingPink;
      *a = GColorImperialPurple; *b = GColorLavenderIndigo;
      break;
    case GEM_TOPAZ:
      *fill = GColorChromeYellow; *hi = GColorPastelYellow;
      *a = GColorWindsorTan; *b = GColorOrange;
      break;
    case GEM_CITRINE:
      *fill = GColorRajah; *hi = GColorIcterine;
      *a = GColorOrange; *b = GColorChromeYellow;
      break;
    case GEM_AQUA:
      *fill = GColorTiffanyBlue; *hi = GColorCeleste;
      *a = GColorBlueMoon; *b = GColorMediumAquamarine;
      break;
    case GEM_GARNET:
      *fill = GColorDarkCandyAppleRed; *hi = GColorMelon;
      *a = GColorBulgarianRose; *b = GColorRed;
      break;
    case GEM_RUBY:
    default:
      *fill = GColorRed; *hi = GColorMelon;
      *a = GColorDarkCandyAppleRed; *b = GColorFashionMagenta;
      break;
  }
#else
  (void)type;
  *fill = GColorBlack;
  *hi = GColorWhite;
  *a = GColorBlack;
  *b = GColorWhite;
#endif
}

void rock_build(RockShape *out, uint8_t stage, uint8_t gem_type,
                uint32_t seed, int cx, int cy, int r, int angle_deg) {
  memset(out, 0, sizeof(*out));
  out->is_gem = (stage >= STAGE_GEM);
  out->facet_a = GColorClear;
  out->facet_b = GColorClear;

  if (out->is_gem) {
    gem_colors(gem_type, &out->fill, &out->highlight, &out->facet_a, &out->facet_b);
    out->edge = GColorBlack;
    out->n = 8;
    static const int8_t gx[] = {0, 5, 7, 5, 0, -5, -7, -5};
    static const int8_t gy[] = {-8, -5, 0, 5, 8, 5, 0, -5};
    for (int i = 0; i < out->n; i++) {
      int x = (gx[i] * r) / 8;
      int y = (gy[i] * r) / 8;
      out->verts[i] = rot(x, y, cx, cy, angle_deg);
    }
    return;
  }

#ifdef PBL_COLOR
  if (stage <= STAGE_ANGULAR) {
    out->fill = GColorDarkGray;
    out->edge = GColorBlack;
    out->highlight = GColorLightGray;
  } else if (stage <= STAGE_SMOOTH) {
    out->fill = GColorLightGray;
    out->edge = GColorDarkGray;
    out->highlight = GColorWhite;
  } else if (stage == STAGE_POLISHED) {
    out->fill = GColorWhite;
    out->edge = GColorDarkGray;
    out->highlight = GColorWhite;
  } else {
    gem_colors(gem_type, &out->fill, &out->highlight, &out->facet_a, &out->facet_b);
    out->edge = GColorBlack;
  }
#else
  out->fill = GColorBlack;
  out->edge = GColorBlack;
  out->highlight = GColorWhite;
#endif

  int n = 10;
  if (stage >= STAGE_ANGULAR) n = 9;
  if (stage >= STAGE_SOFTENED) n = 8;
  if (stage >= STAGE_SMOOTH) n = 7;
  if (stage >= STAGE_POLISHED) n = 6;
  out->n = n;

  int jagged = 5 - (int)stage;
  if (jagged < 0) jagged = 0;

  for (int i = 0; i < n; i++) {
    uint32_t m = mix(seed, (uint32_t)i);
    int base_ang = (360 * i) / n;
    int jitter_ang = jagged ? (int)(m % (uint32_t)(jagged * 8 + 1)) - jagged * 4 : 0;
    int rad_j = jagged ? (int)((m >> 8) % (uint32_t)(jagged * 3 + 1)) : 0;
    int rr = r - jagged + rad_j;
    int ex = (rr * icos(base_ang + jitter_ang)) / 255;
    int ey = (rr * isin(base_ang + jitter_ang)) / 255;
    if (stage >= STAGE_SMOOTH) {
      ey = (ey * 9) / 10;
    }
    out->verts[i] = rot(ex, ey, cx, cy, angle_deg);
  }
}

void rock_draw(GContext *ctx, const RockShape *rock) {
  if (rock->n < 3) {
    return;
  }

  GPathInfo info = {
    .num_points = (uint32_t)rock->n,
    .points = (GPoint *)rock->verts,
  };
  GPath *path = gpath_create(&info);
  if (!path) {
    return;
  }

  graphics_context_set_fill_color(ctx, rock->fill);
  gpath_draw_filled(ctx, path);

  // Facet lines for gems / gleaming stones
  if (rock->is_gem || rock->facet_a.argb != 0) {
    int cx = 0, cy = 0;
    for (int i = 0; i < rock->n; i++) {
      cx += rock->verts[i].x;
      cy += rock->verts[i].y;
    }
    cx /= rock->n;
    cy /= rock->n;
    graphics_context_set_stroke_color(ctx, rock->facet_b);
    for (int i = 0; i < rock->n; i += 2) {
      graphics_draw_line(ctx, GPoint(cx, cy), rock->verts[i]);
    }
    graphics_context_set_stroke_color(ctx, rock->facet_a);
    for (int i = 1; i < rock->n; i += 2) {
      graphics_draw_line(ctx, GPoint(cx, cy), rock->verts[i]);
    }
    graphics_context_set_fill_color(ctx, rock->highlight);
    graphics_fill_circle(ctx, GPoint(cx - 3, cy - 4), rock->is_gem ? 2 : 1);
  } else {
    int cx = 0, cy = 0;
    for (int i = 0; i < rock->n; i++) {
      cx += rock->verts[i].x;
      cy += rock->verts[i].y;
    }
    cx /= rock->n;
    cy /= rock->n;
    graphics_context_set_fill_color(ctx, rock->highlight);
    graphics_fill_circle(ctx, GPoint(cx - 2, cy - 3), 1);
  }

  graphics_context_set_stroke_color(ctx, rock->edge);
  gpath_draw_outline(ctx, path);
  gpath_destroy(path);
}
