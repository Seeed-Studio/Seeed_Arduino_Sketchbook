#define private public
#ifdef max
#pragma push_macro("max")
#pragma push_macro("min")
#undef max
#undef min
#include "algorithm"
#pragma push_macro("max")
#pragma push_macro("min")
#endif
#include <Seeed_FS.h>
#include "SD/Seeed_SD.h"
#include "string.h"
#include "TFT_eSPI.h"
#include "Adafruit_ZeroTimer.h"
#include "Wire.h"
#include "raw.h"
char buf[20];
uint8_t button_state;
#define debug(...)        //\
    Serial.printf(__VA_ARGS__); \
    Serial.println();
#define debug2(...)       //Serial.printf(__VA_ARGS__); Serial.println();
#define debug_begin(baud) //Serial.begin(baud);  while(!Serial);
#define PIN_PLAY BUTTON_3
#define PIN_JUMP BUTTON_2
#define PIN_BREAKING_OUT BUTTON_1

/*****
 * load image from sd 
 ****/
//#define LOAD_IMAGE_SD

//define the interrupt handlers
void TC3_Handler()
{
    Adafruit_ZeroTimer::timerHandler(3);
}

constexpr int16_t screen_width = 320;
constexpr int16_t screen_height = 240;
TFT_eSPI tft;
Adafruit_ZeroTimer zt3 = Adafruit_ZeroTimer(3);

// imgae class
struct raw_image
{
    int16_t width;
    int16_t height;
    uint8_t *ptr()
    {
        return (uint8_t *)(this + 1);
    }
    uint8_t get(int16_t x, int16_t y)
    {
        return this->ptr()[y * width + x];
    }
};

// location for image
enum location : uint8_t
{
    left_top,
    left_bottom,
    right_top,
    right_bottom,
};

struct point
{
    int16_t x;
    int16_t y;
    point() {}
    point(int16_t x, int16_t y) : x(x), y(y)
    {
    }
};

enum pix_type : uint8_t
{
    pix_type_ignore,
    pix_type_actor = 1 << 0,
    pix_type_breaking_out = 1 << 1,
    pix_type_block = 1 << 2,
    pix_type_bird = 1 << 3,
};

struct object : point
{
    raw_image **raw;
    pix_type type;
    location loc;
    uint8_t behavior;
    int8_t x_move_speed;
    int8_t y_move_speed;
    bool in_range;
    object() {}
    object(
        pix_type type,
        point p,
        raw_image **raw,
        int8_t x_move_speed = 0,
        int8_t y_move_speed = 0,
        uint8_t behavior = 0,
        location loc = location::left_bottom) : type(type), raw(raw),
                                                x_move_speed(x_move_speed), y_move_speed(y_move_speed),
                                                behavior(behavior), loc(loc)
    {
        in_range = true;
        set_point(p);
    }
    int16_t width()
    {
        return raw[behavior]->width;
    }
    int16_t height()
    {
        return raw[behavior]->height;
    }
    void move()
    {
        x += x_move_speed;
        y += y_move_speed;
    }
    void set_point(point value)
    {
        *(point *)this = value;
    }
};

struct painter
{
    uint8_t toggle(object &obj, uint8_t a, uint8_t b)
    {
        obj.behavior = obj.behavior == a ? b : a;
        return draw(obj);
    }
    uint8_t draw(object &obj)
    {
        uint8_t mask = 0;
        uint8_t transparent = 0xff;
        raw_image *img = obj.raw[obj.behavior];
        int16_t x_start, x_end, y_start, y_end;
        int16_t x_p = 0, y_p = 0;
        switch (obj.loc)
        {
        case left_top:
            x_start = obj.x;
            y_start = obj.y;
            break;
        case left_bottom:
            x_start = obj.x;
            y_start = obj.y - img->height;
            break;
        case right_top:
            x_start = obj.x - img->width;
            y_start = obj.y;
            break;
        case right_bottom:
        default:
            x_start = obj.x - img->width;
            y_start = obj.y - img->height;
            break;
        }

        x_end = x_start + img->width;
        y_end = y_start + img->height;

        if (
            not_in_range(x_start, y_start) &&
            not_in_range(x_start, y_end) &&
            not_in_range(x_end, y_start) &&
            not_in_range(x_end, y_end))
        {
            obj.in_range = false;
            debug("not in range");
            return mask;
        }
        if (x_start < 0)
        {
            x_p = -x_start;
            x_start = 0;
        }
        if (x_end > screen_width)
        {
            x_end = screen_width;
        }
        if (y_start < 0)
        {
            y_p = -y_start;
            y_start = 0;
        }
        if (y_end > screen_height)
        {
            y_end = screen_height;
        }
        for (uint16_t y = y_start, x_tmp = x_p; y < y_end; y++, y_p++)
        {
            for (uint16_t x = x_start, x_p = x_tmp; x < x_end; x++, x_p++)
            {
                auto value = img->get(x_p, y_p);
                if (value == transparent)
                {
                    continue;
                }

                buffer[y][x] = value;

                if (obj.type == pix_type_ignore)
                {
                    continue;
                }
                if (collide[y][x] == pix_type_ignore)
                {
                    collide[y][x] = obj.type;
                }
                else if (collide[y][x] != obj.type)
                {
                    mask |= collide[y][x];
                }
            }
        }
        return mask;
    }
    void clean()
    {
        memset(collide, pix_type_ignore, sizeof(collide));
        memset(buffer, -1, sizeof(buffer));
    }
    void flush()
    {
        tft.pushImage(0, 0, screen_width, screen_height, (uint8_t *)buffer);
    }

private:
    bool not_in_range(int16_t x, int16_t y)
    {
        return !(0 <= x && x < screen_width && 0 <= y && y < screen_height);
    }
    uint8_t buffer[screen_height][screen_width];
    pix_type collide[screen_height][screen_width];
};

volatile bool pushed_play = false;
volatile bool pushed_jump = false;
volatile bool pushed_breaking_out = false;

void push_jump()
{
    if (digitalRead(PIN_JUMP) == LOW)
    {
        pushed_jump = true;
    }
}

void push_breaking_out()
{
    if (digitalRead(PIN_BREAKING_OUT) == HIGH)
    {
        pushed_breaking_out = true;
    }
}
void push_play()
{
    if (digitalRead(PIN_PLAY) == HIGH)
    {
        pushed_play = true;
    }
}

struct jumper
{
private:
    enum class game
    {
        over,
        go_on,
    };
    enum speed
    {
        x_bird_speed = -10,
        x_block_speed = -10,
        x_cloud_speed = -2,
        x_breaking_out_speed = 24,
    };
    enum hung_raw
    {
        jump0,
        jump1,
        jump2,
        jump3,
        jump4,
        jump5,
        jump6,
        fall0,
        fall1,
        fall2,
        fall3,
        fall4,
        fall5,
        max_jump_path,
    };
    enum
    {
        max_wire_count = 1,
        max_bird_count = 1,
        max_block_count = 3,
        max_cloud_count = 3,
        max_count_down = 4,
        max_number_count = 10,
        max_road_count = 11,
        max_object_count = 20,
        max_point_count = 6,
    };
    enum actor_behavior
    {
        run0,
        run1,
        run_mode = run1,
        breaking_out,
        jump,
        failure,
        max_actor_behavior,
    };
    enum bird_behavior
    {
        fly0,
        fly1,
        max_bird_behavior,
    };
    enum block_behavior
    {
        small_block,
        big_block,
        max_block_type,
    };
    raw_image *raw_actor[max_actor_behavior];
    raw_image *raw_bird[max_bird_behavior];
    raw_image *raw_block[max_block_type];
    raw_image *raw_cloud[1];
    raw_image *raw_game_over[1];
    raw_image *raw_wire[1];
    raw_image *raw_road[1];
    raw_image *raw_wait_play[1];
    raw_image *raw_setup[1];
    raw_image *raw_number[max_number_count];
    raw_image *raw_count_down[max_count_down];
    painter pan;
    object actor;
    object game_over;
    object wire[1];
    object point_group[max_point_count];
    object road[max_road_count];
    object bird[max_bird_count];
    object block[max_block_count];
    object cloud[max_cloud_count];
    object over;
    point jump_path[max_jump_path];
    point start_of_block;
    point start_of_breaking_out;
    int8_t actor_last_behavior;
    int8_t current_bird_count;
    int8_t current_block_count;
    int8_t current_cloud_count;
    int8_t current_wire_count;
    int8_t current_breaking_out_count;
    int8_t current_jump_state;
    uint32_t current_point;

public:
    void begin()
    {
        debug("load image");
#ifdef LOAD_IMAGE_SD
        raw_actor[run0] = load_image("rgb332/run0.bmp");
        raw_actor[run1] = load_image("rgb332/run1.bmp");
        raw_actor[jump] = load_image("rgb332/jump.bmp");
        raw_actor[breaking_out] = load_image("rgb332/breaking_out.bmp");
        raw_actor[failure] = load_image("rgb332/failure.bmp");
        raw_block[small_block] = load_image("rgb332/small_block.bmp");
        raw_block[big_block] = load_image("rgb332/big_block.bmp");
        raw_bird[fly0] = load_image("rgb332/fly0.bmp");
        raw_bird[fly1] = load_image("rgb332/fly1.bmp");
        raw_cloud[0] = load_image("rgb332/cloud.bmp");
        raw_wire[0] = load_image("rgb332/wire.bmp");
        raw_road[0] = load_image("rgb332/road.bmp");
        raw_count_down[0] = load_image("rgb332/go.bmp");
        raw_count_down[1] = load_image("rgb332/big_1.bmp");
        raw_count_down[2] = load_image("rgb332/big_2.bmp");
        raw_count_down[3] = load_image("rgb332/big_3.bmp");
        raw_number[0] = load_image("rgb332/0.bmp");
        raw_number[1] = load_image("rgb332/1.bmp");
        raw_number[2] = load_image("rgb332/2.bmp");
        raw_number[3] = load_image("rgb332/3.bmp");
        raw_number[4] = load_image("rgb332/4.bmp");
        raw_number[5] = load_image("rgb332/5.bmp");
        raw_number[6] = load_image("rgb332/6.bmp");
        raw_number[7] = load_image("rgb332/7.bmp");
        raw_number[8] = load_image("rgb332/8.bmp");
        raw_number[9] = load_image("rgb332/9.bmp");
        raw_game_over[0] = load_image("rgb332/game_over.bmp");
        raw_wait_play[0] = load_image("rgb332/wait_play.bmp");
#else
        raw_actor[run0] = (raw_image *)RUN0;                 //load_image("rgb332/run0.bmp")
        raw_actor[run1] = (raw_image *)RUN1;                 //load_image("rgb332/run1.bmp");
        raw_actor[jump] = (raw_image *)JUMP;                 //load_image("rgb332/jump.bmp");
        raw_actor[breaking_out] = (raw_image *)BREAKING_OUT; //l("rgb332/breaking_out.bmp");
        raw_actor[failure] = (raw_image *)FAILURE;           //load_image("rgb332/failure.bmp");
        raw_block[small_block] = (raw_image *)SMALL_BLOCK;   //load_image("rgb332/small_block.bmp");
        raw_block[big_block] = (raw_image *)BIG_BLOCK;       //load_image("rgb332/big_block.bmp");
        raw_bird[fly0] = (raw_image *)FLY0;                  //load_image("rgb332/fly0.bmp");
        raw_bird[fly1] = (raw_image *)FLY1;                  //load_image("rgb332/fly1.bmp");
        raw_cloud[0] = (raw_image *)CLOUD;                   //load_image("rgb332/cloud.bmp");
        raw_wire[0] = (raw_image *)WIRE_BMP;                 //load_image("rgb332/wire.bmp");
        raw_road[0] = (raw_image *)ROAD;                     //load_image("rgb332/road.bmp");
        raw_count_down[0] = (raw_image *)GO;                 //load_image("rgb332/go.bmp");
        raw_count_down[1] = (raw_image *)BIG_1;              //load_image("rgb332/big_1.bmp");
        raw_count_down[2] = (raw_image *)BIG_2;              //load_image("rgb332/big_2.bmp");
        raw_count_down[3] = (raw_image *)BIG_3;              //load_image("rgb332/big_3.bmp");
        raw_number[0] = (raw_image *)_0BMP;                  //load_image("rgb332/0.bmp");
        raw_number[1] = (raw_image *)_1BMP;                  //load_image("rgb332/1.bmp");
        raw_number[2] = (raw_image *)_2BMP;                  //load_image("rgb332/2.bmp");
        raw_number[3] = (raw_image *)_3BMP;                  //load_image("rgb332/3.bmp");
        raw_number[4] = (raw_image *)_4BMP;                  //load_image("rgb332/4.bmp");
        raw_number[5] = (raw_image *)_5BMP;                  //load_image("rgb332/5.bmp");
        raw_number[6] = (raw_image *)_6BMP;                  //load_image("rgb332/6.bmp");
        raw_number[7] = (raw_image *)_7BMP;                  //load_image("rgb332/7.bmp");
        raw_number[8] = (raw_image *)_8BMP;                  //load_image("rgb332/8.bmp");
        raw_number[9] = (raw_image *)_9BMP;                  //load_image("rgb332/9.bmp");
        raw_game_over[0] = (raw_image *)GAME_OVER;           //load_image("rgb332/game_over.bmp");
        raw_wait_play[0] = (raw_image *)WAIT_PLAY;           //("rgb332/wait_play.bmp");
#endif

        raw_setup[0] = (raw_image *)SETUP; // raw_setup[0] = load_image("rgb332/setup.bmp"); // to lager for load from SD Card
        debug("finish load");
    }

    void play()
    {
        game flag = game::go_on;
        reset();
        while (flag != game::over)
        {
            pan.clean();
            calc();
            frame(flag);
            pan.flush();
        }
        do
        {
            show_game_over();
            delay(30);
        } while (!pushed_play);
    }

private:
    void frame(game &flag)
    {
        uint8_t collide_mask = 0;
        flag = game::go_on;
        point point_pos(screen_width - 10, point_group[0].height() + 10);
        for (size_t i = 0; i < max_point_count; i++)
        {
            object p = point_group[i];
            point_pos.x -= p.width();
            p.set_point(point_pos);
            pan.draw(p);
        }
        for (size_t i = 0; i < max_road_count; i++)
        {
            pan.draw(road[i]);
        }
        for (size_t i = 0; i < current_cloud_count; i++)
        {
            pan.draw(cloud[i]);
        }
        if (actor.behavior <= run_mode)
        {
            pan.toggle(actor, run0, run1);
        }
        else
        {
            pan.draw(actor);
            if (actor.behavior == breaking_out)
            {
                actor.behavior = actor_last_behavior;
            }
        }
        for (size_t i = 0; i < current_block_count; i++)
        {
            collide_mask |= pan.draw(block[i]);
        }
        for (size_t i = 0; i < current_bird_count; i++)
        {
            collide_mask |= pan.toggle(bird[i], fly0, fly1);
        }
        if (collide_mask & pix_type_actor)
        {
            flag = game::over;
        }
        for (size_t i = 0; i < current_wire_count; i++)
        {
            if (pan.draw(wire[i]) & pix_type_bird)
            {
                debug("breaking it");
                bird->in_range = false;
                wire[i].in_range = false;
            }
        }
    }
    void calc()
    {
        auto rate = 10;
        auto can_generate = [this](object *obj, uint32_t current_count) {
            if (current_count > 0)
            {
                auto &o = obj[current_count - 1];
                auto b = o.raw[o.behavior];
                return o.x < screen_width - actor.raw[actor.behavior]->width * 5;
            }
            else
            {
                return true;
            }
        };
        if (pushed_jump)
        {
            actor.behavior = jump;
        }
        if (pushed_breaking_out)
        {
            pushed_breaking_out = false;
            if (current_wire_count == 0)
            {
                current_wire_count = 1;
                actor_last_behavior = actor.behavior;
                actor.behavior = breaking_out;
                wire[0] = object(
                    pix_type_breaking_out,
                    point(actor.x + raw_actor[0]->width, actor.y - 16),
                    raw_wire,
                    x_breaking_out_speed);
                debug("generate wire");
            }
        }
        if (actor.behavior == jump)
        {
            actor.set_point(jump_path[current_jump_state++]);
            if (current_jump_state == max_jump_path)
            {
                current_jump_state = 0;
                actor.behavior = run0;
                pushed_jump = false;
            }
        }

        //generate a cloud
        if (current_cloud_count < max_cloud_count && can_generate(cloud, current_cloud_count))
        {
            debug("generate a cloud");
            cloud[current_cloud_count++] = object(
                pix_type_ignore,
                point(screen_width - 1, random(60, 100)),
                raw_cloud,
                x_cloud_speed);
        }

        //generate a block
        bool has_generate_block = current_block_count == 0;
        if (current_block_count < max_block_count)
        {
            has_generate_block = can_generate(block, current_block_count);
            if (has_generate_block && random(0, rate) == 0)
            {
                debug("generate a block");
                block[current_block_count++] = object(
                    pix_type_block,
                    start_of_block,
                    raw_block,
                    x_block_speed, 0,
                    random(0, 4) ? small_block : big_block);
            }
        }

        //generate a bird
        static bool once = true;
        if (once)
        {
            once = false;
            int i = 0;
            while (current_bird_count < max_bird_count)
            {
                bird[current_bird_count++] = object(
                    pix_type_bird,
                    point(screen_width - raw_bird[0]->width, jump_path[i].y - 16),
                    raw_bird,
                    x_bird_speed);
                i++;
            }
        }
        else if (
            current_bird_count < max_bird_count &&
            random(0, rate) == 0 &&
            can_generate(bird, current_bird_count))
        {
            debug("generate a bird");
            auto i = random(0, max_jump_path);
            bird[current_bird_count++] = object(
                pix_type_bird,
                point(screen_width - 1, jump_path[i].y - 16),
                raw_bird,
                x_bird_speed);
        }
        for (size_t i = 0, v = current_point / 10; i < max_point_count; i++, v /= 10)
        {
            point_group[i].behavior = v % 10;
        }
        if (move(road, max_road_count) != max_road_count)
        {
            road[max_road_count - 1] = road[max_road_count - 2];
            road[max_road_count - 1].x += raw_road[0]->width - 1;
        }
        current_point += 1;
        current_bird_count = move(bird, current_bird_count);
        current_wire_count = move(wire, current_wire_count);
        current_cloud_count = move(cloud, current_cloud_count);
        current_block_count = move(block, current_block_count);
    }
    void reset()
    {
        debug("reset variable");
        size_t start_height = 180;
        for (
            size_t i = 0, a = 7, step = a * max_jump_path / 2, h = start_height;
            i < max_jump_path;
            i++, step -= a, h -= step)
        {
            jump_path[i] = point(40, h);
        }
        for (size_t i = 0, x = 1; i < max_road_count; i++, x += raw_road[0]->width)
        {
            road[i] = object(pix_type_ignore, point(x - 1, screen_height - 1), raw_road, x_block_speed);
        }
        for (size_t i = 0; i < max_point_count; i++)
        {
            point_group[i] = object(
                pix_type_ignore,
                point(screen_width - 10, 10),
                raw_number);
        }
        for (
            size_t i = 0, w = 30;
            i < max_cloud_count;
            i++, w += (screen_width) / max_cloud_count)
        {
            cloud[i] = object(
                pix_type_ignore,
                point(w, random(60, 100)),
                raw_cloud,
                x_cloud_speed);
        }

        actor = object(pix_type_actor, jump_path[0], raw_actor);
        current_point = 0;
        current_bird_count = 0;
        current_block_count = 0;
        current_cloud_count = 0;
        current_jump_state = 0;
        current_wire_count = 0;
        current_cloud_count = max_cloud_count;
        start_of_block = point(screen_width - 1, start_height);
        debug("finish reset");

        while (!pushed_play)
        {
            pan.clean();
            point center(
                (screen_width - raw_wait_play[0]->width) / 2,
                (screen_height + raw_wait_play[0]->height) / 2);
            object wait(
                pix_type_ignore,
                center,
                raw_wait_play);
            point left_top1(0, 0);
            object setup1(
                pix_type_ignore,
                left_top1,
                raw_setup);
            setup1.loc = location::left_top;
            pan.draw(wait);
            pan.flush();
            delay(500);
            pan.draw(setup1);
            pan.flush();
            delay(500);
        }

        pushed_play = false;

        //count down
        for (size_t i = 4; i-- > 0;)
        {
            pan.clean();
            point center(
                (screen_width - raw_count_down[i]->width) / 2,
                (screen_height + raw_count_down[i]->height) / 2);
            object n(
                pix_type_ignore,
                center,
                raw_count_down + i);
            pan.draw(n);
            pan.flush();
            delay(400);
        }
    }
    void show_game_over()
    {
        // Serial.print("cc: ");
        // Serial.println(current_point);
        game flag;

        point center(
            (screen_width - raw_game_over[0]->width) / 2,
            (screen_height + raw_game_over[0]->height) / 2);
        object o(pix_type_ignore, center, raw_game_over);
        actor.behavior = failure;
        pan.clean();
        frame(flag);
        pan.draw(o);
        pan.flush();
        pan.clean();
        delay(200);
        frame(flag);
        pan.flush();
        delay(200);
    }
    void turn_light_on()
    {
    }
    void turn_light_off()
    {
    }
    raw_image *load_image(const char *path)
    {
        File f = SD.open(path, FILE_READ);
        if (!f)
        {
            debug("read file error");
            return nullptr;
        }
        int32_t size = f.size();
        raw_image *mem = (raw_image *)new uint8_t[size];
        f.read(mem, size);
        f.close();
        debug(path);
        return mem;
    }
    int8_t move(object *obj, int8_t count)
    {
        uint8_t in_range[max_object_count];
        uint8_t end = 0;

        //keep the object index which in range.
        for (size_t i = 0; i < count; i++)
        {
            obj[i].move();
            if (obj[i].in_range)
            {
                in_range[end++] = i;
            }
        }

        //compress
        for (size_t i = 0; i < end; i++)
        {
            auto index = in_range[i];
            if (i == index)
            {
                continue;
            }
            obj[i] = obj[index];
        }
        return end;
    }
};

jumper game;

void setup()
{
    debug_begin(115200);
#ifdef LOAD_IMAGE_SD
    while (!SD.begin(SDCARD_SS_PIN, SDCARD_SPI, 4000000UL))
    {
        debug("Card Mount Failed");
        return;
    }
    debug("SD Card Mount Successful!");
#endif
    tft.init();
    tft.setRotation(3);
    tft.fillScreen(TFT_WHITE);

    zt3.configure(TC_CLOCK_PRESCALER_DIV1024, TC_COUNTER_SIZE_16BIT, TC_WAVE_GENERATION_MATCH_FREQ);
    zt3.setCompare(0, 920 * 2); //1024div + 920 -> 50hz -> 25hz
    zt3.setCallback(true, TC_CALLBACK_CC_CHANNEL0, push_jump);
    zt3.enable(true);

    pinMode(BUZZER_CTR, OUTPUT);

    pinMode(PIN_PLAY, INPUT);
    pinMode(PIN_JUMP, INPUT_PULLUP);
    pinMode(PIN_BREAKING_OUT, INPUT);
    attachInterrupt(digitalPinToInterrupt(PIN_PLAY), push_play, CHANGE);
    attachInterrupt(digitalPinToInterrupt(PIN_BREAKING_OUT), push_breaking_out, CHANGE);
    randomSeed(analogRead(A0));
    game.begin();
}
void loop()
{

    game.play();
}
